#!/usr/bin/python3

opt_option=
root_dir=

import os, sys, re, json
sys.path.append("{}/test".format(root_dir))
import compare
from compilerbugs import Bug, Compiler, CompilerType, pintool
from enum import Enum
from itertools import combinations
import multiprocessing as mp

default_gccoptions_path = "./gccoptions.json"
default_gccmaxlength_comb = 3

compiler_options_map = {}


def getindent(line:str):
    res = 0
    for c in line:
        if not c.isspace():
            return res
        else:
            res += 1

class Option:
    def __init__(self, name:str = "") -> None:
        self.name = name
        self.children = []
        self.fa = None
        self.valid = True

    def __hash__(self) -> int:
        return hash(self.name)
    
    def __eq__(self, option) -> bool:
        return self.name == option.name

    def __str__(self) -> str:
        return self.name
    
    def __repr__(self) -> str:
        return self.name


def optiter_to_str(optlist):
    return ' '.join([opt.name for opt in optlist])


def select_block_options(compiler_name, choices, visit, i):
    workdir = "{}/regression/select/{}".format(root_dir, i)
    os.system("mkdir -p {}".format(workdir))
    os.chdir(workdir)
    os.system("cp ../output.c ./output.c")
    
    result = []
    for choice in choices:
        if visit.intersection(choice) != set():
            continue
        option_names = [opt.name for opt in choice]
            
        os.system("{} ./output.c {} {} -o output 2>/dev/null 1>&2".format(compiler_name, opt_option, ' '.join(option_names)))
        pintool("./output")
        res = os.popen("cat result.out").read().strip()
        if res == "0":
            result.append(choice)
            visit.update(choice)
    
    return result

class OptionMgr:
    compiler_mgr_map = {}
    @classmethod
    def get_optionmgr(cls, name):
        if name not in cls.compiler_mgr_map:
            optionMgr = OptionMgr(name)
            if optionMgr.compilerType == CompilerType.clang:
                optionMgr.get_clang_options(name)
            elif optionMgr.compilerType == CompilerType.gcc:
                optionMgr.get_gcc_options(name)
            cls.compiler_mgr_map[name] = optionMgr
        return cls.compiler_mgr_map[name]


    def __init__(self, compiler_name:str = "") -> None:
        self.compiler_name = compiler_name
        
        if "gcc" in compiler_name:
            self.compilerType = CompilerType.gcc
        elif "clang" in compiler_name:
            self.compilerType = CompilerType.clang
        else:
            assert(0)
        
        self.roots = []     # single option or base option
        
        self.sequence = []  # original arguments
        self.checked = False
        self.options = set()    # all options set
    

    def get_clang_options(self, compiler_name):
        headerFeat = "Pass Arguments:"

        tempdir = os.popen("mktemp -d").read().strip()
        oldpath = os.getcwd()
        os.chdir(tempdir)
        # create null c file
        with open("c.c", "w") as cfile:
            cfile.writelines(["int main (){\n", "\treturn 0;\n", "}"])
        os.system("{} c.c {} -mllvm -debug-pass=Arguments 2>{}.arguments".format(compiler_name, opt_option, compiler_name))
        with open("{}.arguments".format(compiler_name), "r") as clangf:
            lines = clangf.readlines()
        
        for line in lines:
            line = line.replace(headerFeat, "", 1)
            line_option_names = line.strip().split()
            line_options = [Option(name) for name in line_option_names]

            self.roots.extend(line_options)
            self.sequence.extend(line_options)
            self.options.update(line_options)
        
        os.chdir(oldpath)
    
    def get_gcc_options(self, compiler_name:str):
        
        with open(default_gccoptions_path, "r") as opt_f:
            options_dict = json.load(opt_f)
        if compiler_name not in options_dict:
            print("can't find compiler options of {}".format(compiler_name))
        option_names = options_dict[compiler_name]
        for option_name in option_names:
            # add the -fno- flag
            if not option_name.startswith("-f"):
                continue
            if "=" in option_name:
                continue
            self.roots.append(Option("-fno-" + option_name[2:]))


        
        return
    
        tempdir = os.popen("mktemp -d").read().strip()
        oldpath = os.getcwd()
        os.chdir(tempdir)
        
        # create null c file
        with open("c.c", "w") as cfile:
            cfile.writelines(["int main (){\n", "\treturn 0;\n", "}"])
        os.system("{} c.c {} -fdump-passes 2>{}.passes".format(compiler_name, opt_option, compiler_name))
        with open("{}.passes".format(compiler_name), "r") as gccf:
            lines = gccf.readlines()
            lines = [line.rstrip() for line in lines]
        
        ''' assume structure info is indented with 2 whitespaces for each level
        '''

        stack = []  # Elem Type: tuple(indent, option)
        for line in lines:
            if line.lstrip().startswith("*"):
                continue            

            else:
                indent = getindent(line)
                items = line.split(":")
                assert(len(items) == 2)
                
                if not "ON" in items[1]:
                    continue

                while len(stack) > 0 and stack[-1][0] >= indent:
                    stack.pop()
                
                option = Option("-fenable-" + items[0].strip())
                print("{} \t{}".format(option.name, line.strip()))
                if len(stack) > 0:
                    stack[-1][1].children.append(option)
                    option.fa = stack[-1][1]
                else:
                    self.roots.append(option)
                
                stack.append((indent, option))                        

                self.options.add(option)
        
        os.chdir(oldpath)


    ''' expand a base option with all possible option sequence
    '''
    @classmethod
    def expand(cls, option:Option) -> list:
        res = []
        if not option.children:
            return [[option]]
        
        for child in option.children:
            child_list = cls.expand(child)
            for child_option_seq in child_list:
                res.append([option] + child_option_seq)
        
        return res

    ''' init all validation settings
    '''
    def refresh_valid(self):
        self.validseq = []
        for root in self.roots:
            stack = [root]
            while stack:
                option = stack.pop()
                option.valid = True
                for child in option.children:
                    stack.append(child)

    ''' check validation of each option, return a validated full option sequence
        (used to check whether a bug still can be triggered)

        pre-condition: get_clang|gcc_options() has been invoked
    '''
    def checkvalidation(self, test_file_path:str = "", force:bool = False) -> list:
        if self.checked and not force:
            return
        
        if force:
            self.refresh_valid()

        tempdir = os.popen("mktemp -d").read().strip()
        oldpath = os.getcwd()
        
        # preparation
        if self.compilerType == CompilerType.gcc:
            # prepare test C file
            if not test_file_path:
                with open("c.c", "w") as cfile:
                    cfile.writelines(["int main (){\n", "\treturn 0;\n", "}"])
            else:
                os.system("cp {} {}/c.c".format(test_file_path, tempdir))
            
        elif self.compilerType == CompilerType.clang:
            # get opt info
            optallstr = os.popen("{} --help".format(self.compiler_name.replace("clang", "opt"))).read()
        
        os.chdir(tempdir)

        for root in self.roots:
            cur_option_seq_list = OptionMgr.expand(root)
            for seq in cur_option_seq_list:
                seq_str = ' '.join([option.name for option in seq])
                
                if self.compilerType == CompilerType.gcc:
                    fail = os.system("{} c.c {} {} 2>/dev/null 1>&2".format(self.compiler_name, seq_str, opt_option))
                elif self.compilerType == CompilerType.clang:
                    fail = os.system("llvm-as </dev/null | {} {} -disable-output 2>/dev/null".format(self.compiler_name.replace("clang", "opt"), seq_str))
                    if not fail:
                        fail = not seq_str in optallstr

                if fail:
                    seq[-1].valid = False
                
        os.chdir(oldpath)
        self.checked = True
    
    ''' get the valid full sequence
    
    '''
    def getseq(self, wantvalid = True):
        seq = []
        
        for root in self.roots:
            stack = [root]
            while len(stack) > 0:
                option = stack.pop()
                if option.valid == wantvalid:
                    seq.append(option)
                for child in option.children:
                    stack.append(child)

        return seq


    ''' select the minimum option-sequence set that can block the introduction of load
    '''
    def select_gcc(self, bug, n = 1):
        '''
        1) remove options that may bring runtime error to program, get a sequence of length n
        2) try to block problem with disjoint `-fno-` option combinations of size from 1 to 3
        '''

        workdir = "{}/regression/select".format(root_dir)
        os.system("mkdir -p {}".format(workdir))
        os.chdir(workdir)
        os.system("cp {}/regression/caserepo/output{}.c ./output.c".format(root_dir, bug.id))

        print("bug {}".format(bug.id))
        # 1)
        self.checkvalidation("./output.c", True)
        full_seq = self.getseq(True)
        invalid_seq = self.getseq(False)
        
        print("#phase 1:    filter valid ({}) options\n".format(len(full_seq)))
        print("valid: {}".format(full_seq))
        print("these are invalid {}\n".format(invalid_seq))
        
        # 2)
        
        visit = set()
        result = []
        for size in range(1, default_gccmaxlength_comb + 1):
            
            res_list = []
            pool = mp.Pool(n)
            choices = list(combinations(full_seq, size))
            print("... testing {} choices of {} -fno- options".format(len(choices), size))
            choices_list = [[] for i in range(n)]
            for i, choice in enumerate(choices):
                choices_list[i % n].append(choice)
            
            for i in range(n):
                res_list.append(pool.apply_async(select_block_options, (self.compiler_name, choices_list[i], visit, i,)))
            
            pool.close()
            pool.join()
            
            for res in res_list:
                opt_comb_list = res.get()
                visit.update(*opt_comb_list)
                result.extend(opt_comb_list)

                
        print("#phase 2:    {} options group can block this optimization problem".format(len(result)))
        for choice in result:
            option_names = [opt.name for opt in choice]
            print(option_names)
        print()
        return result



if __name__ == "__main__":
    caseid = int(sys.argv[1])
    bug = Bug(caseid)
    optionmgr = OptionMgr.get_optionmgr(sys.argv[2])
    optionmgr.select_gcc(bug, 24)