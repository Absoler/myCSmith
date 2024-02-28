#!/usr/bin/python3

default_option=
root_dir=

import os, sys, re, json, random
from math import ceil
sys.path.append("{}/test".format(root_dir))
from compilerbugs import Bug, CompilerType, pintool, Compiler, compile
import compare
from itertools import combinations
import multiprocessing as mp
from enum import Enum
class OptionType(Enum):
    f = 0,
    fno = 1,
    main = 2

default_gccoptions_path = "{}/regression/gccoptions.json".format(root_dir)
default_clangoptions_path = "{}/regression/clangoptions.json".format(root_dir)
default_maxlength_comb = 2
default_heuristic_limit = 100

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


def select_block_options(compiler_name:str, oldinfo, choices:list, visit:set, i:int):
    workdir = "{}/regression/select/{}".format(root_dir, i)
    os.system("mkdir -p {}".format(workdir))
    os.chdir(workdir)
    os.system("cp ../output.c ./output.c")
    
    result = []
    for choice in choices:
        if visit.intersection(choice) != set():
            continue
            
        ret = compile(compiler_name, optiter_to_str(choice), "./output.c", "output")
        res = pintool("./output")
        info = compare.parse()
        if ret == 0 and oldinfo != info:
            result.append(choice)
            visit.update(choice)
    
    return result

def select_trigger_options(compiler_name:str, oldinfo, choices:list, i:int):
    workdir = "{}/regression/select/{}".format(root_dir, i)
    os.system("mkdir -p {}".format(workdir))
    os.chdir(workdir)
    os.system("cp ../output.c ./output.c")
    
    result = []
    for choice in choices:
        is_main_opt = True if choice[0].startswith("-O") else False

        ret = os.system("{} ./output.c -I{}/runtime/ {} {} -o output 2>/dev/null 1>&2".format(compiler_name, root_dir, "" if is_main_opt else default_option, ' '.join(choice)))
        # print("{} ./output.c -I{}/runtime/ {} {} -o output 2>/dev/null 1>&2".format(compiler_name, root_dir, default_option, ' '.join(choice)))
        res = pintool("./output")
        info = compare.parse()
        if ret == 0 and oldinfo == info:
            result.append(choice)
    
    return result

def select_heuristically(compiler_name:str, oldinfo, opt_list:list, limit:int, i:int):
    workdir = "{}/regression/select/{}".format(root_dir, i)
    os.system("mkdir -p {}".format(workdir))
    os.chdir(workdir)
    os.system("cp ../output.c ./output.c")


    choice = []
    live_inds = [j for j in range(len(opt_list))]
    result = []
    for j in range(limit):
        
        while True:
            choice = []
            can_del = False
            while not can_del:
                live_inds_copy = live_inds.copy()
                del_ind = random.choice(live_inds_copy)
                live_inds_copy.remove(del_ind)
                for live_ind in live_inds:
                    if live_ind != del_ind:
                        choice.append(opt_list[live_ind])

                ret = compile(compiler_name, optiter_to_str(choice), "./output.c", "output")
                res = pintool("./output")
                info = compare.parse()
                if ret == 0 and oldinfo != info:
                    can_del = True
                    live_inds.remove(del_ind)
                if len(live_inds_copy) == 0:
                    break
            
            if not can_del:
                # fno sequence fail to reduce continually
                result.append(list(map(opt_list.__getitem__, live_inds)))
                break
    
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
        self.using_opt = False
    

    def get_clang_options(self, compiler_name):
        with open(default_clangoptions_path, "r") as opt_f:
            options_dict = json.load(opt_f)
        if compiler_name not in options_dict:
            print("can't find compiler options of {}".format(compiler_name))
        options_dict = options_dict[compiler_name]

        if "clang" in options_dict:
            clang_option_names = options_dict["clang"]
            for option_name in clang_option_names:
                self.roots.append(Option(option_name))
        if "Xclang" in options_dict:
            Xclang_option_names = options_dict["Xclang"]
            for option_name in Xclang_option_names:
                self.roots.append(Option("-Xclang " + option_name))
        if "mllvm" in options_dict:
            mllvm_option_names = options_dict["mllvm"]
            for option_name in mllvm_option_names:
                self.roots.append(Option("-mllvm " + option_name))

        return
        headerFeat = "Pass Arguments:"

        tempdir = os.popen("mktemp -d").read().strip()
        oldpath = os.getcwd()
        os.chdir(tempdir)
        # create null c file
        with open("c.c", "w") as cfile:
            cfile.writelines(["int main (){\n", "\treturn 0;\n", "}"])
        os.system("{} c.c {} -mllvm -debug-pass=Arguments 2>{}.arguments".format(compiler_name, default_option, compiler_name))
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
        os.system("{} c.c {} -fdump-passes 2>{}.passes".format(compiler_name, default_option, compiler_name))
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
        if self.compilerType == CompilerType.gcc or not self.using_opt:
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
                
                if self.compilerType == CompilerType.gcc or not self.using_opt:
                    fail = compile(self.compiler_name, seq_str, "c.c")
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
        2) try to block problem with disjoint `-fno-` option combinations of size from 1 to `default_maxlength_comb`
        3) try to remove useless option from full sequence, selection is random, do `default_heuristic_limit` times
        '''

        workdir = "{}/regression/select".format(root_dir)
        os.system("mkdir -p {}".format(workdir))
        os.chdir(workdir)
        os.system("cp {}/regression/caserepo/output{}.c ./output.c".format(root_dir, bug.id))
        compile(self.compiler_name, "", "./output.c", "output")
        pintool("./output")
        oldinfo = compare.parse()

        print("bug {}".format(bug.id))
        # 1)
        self.checkvalidation("./output.c", True)
        full_seq = self.getseq(True)
        invalid_seq = self.getseq(False)
        ret = compile(self.compiler_name, optiter_to_str(full_seq), "./output.c", "output")
        res = pintool("./output")
        info = compare.parse()
        if ret != 0 or info == oldinfo:
            print("even all fno options can't block the bug {}".format(bug.id))
            return []
        
        print("#phase 1:    filter valid ({}) options\n".format(len(full_seq)))
        print("valid: {}".format(full_seq))
        print("these are invalid {}\n".format(invalid_seq))
        
        # 2)
        
        visit = set()
        result = []
        for size in range(1, default_maxlength_comb + 1):
            
            res_list = []
            pool = mp.Pool(n)
            choices = list(combinations(full_seq, size))
            print("... testing {} choices of {} -fno- options".format(len(choices), size))
            choices_list = [[] for i in range(n)]
            for i, choice in enumerate(choices):
                choices_list[i % n].append(choice)
            
            for i in range(n):
                res_list.append(pool.apply_async(select_block_options, (self.compiler_name, oldinfo, choices_list[i], visit, i,)))
            
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

        # 3)
        # select_heuristically(self.compiler_name, oldinfo, full_seq, 1, 0)
        heuristic_result = []
        pool = mp.Pool(n)
        res_list = []
        for i in range(n):
            res_list.append(pool.apply_async(select_heuristically, (self.compiler_name, oldinfo, full_seq, int(default_heuristic_limit/n), i)))

        pool.close()
        pool.join()

        for res in res_list:
            heuristic_result.extend(res.get())
        heuristic_result = list(set([tuple(seq) for seq in heuristic_result]))
        heuristic_result.sort(key=lambda l : len(l))
        print("#phase 3:    try {} times heuristic reduction and choose the minimum 5% sequences".format(default_heuristic_limit))
        for res in heuristic_result:
            option_names = [opt.name for opt in res]
            print(option_names)
        result.extend(heuristic_result[:ceil(default_heuristic_limit/20)])
        return result

''' try select the option combinations that can trigger this bug on other compiler versions
    1) get the oracle `info` and compilers and their options to be tested
    2) test their combinations from 1 to default_maxlength_comb
'''
def select_migration(bug, option_type, n = 1):
    
    if bug.compilerType != CompilerType.gcc:
        return
    # 1)
    workdir = "{}/regression/select".format(root_dir)
    os.system("mkdir -p {}".format(workdir))
    os.chdir(workdir)
    os.system("cp {}/regression/caserepo/output{}.c ./output.c".format(root_dir, bug.id))
    oldinfo = bug.info
    existed_compiler_names = Compiler.uncompress(bug.compilerHash, CompilerType.gcc)
    
    with open(default_gccoptions_path, "r") as opt_f:
        options_dict = json.load(opt_f)
    for existed_compiler_name in existed_compiler_names:
        options_dict.pop(existed_compiler_name)
    
    if len(options_dict) == 0:
        return
    
    # 2)
    result = {}
    print("... testing {}".format(bug.id))
    
    for compiler_name in options_dict:
        exist = os.system("which {} > /dev/null".format(compiler_name))
        if exist != 0:
            continue
        res_list = []
        pool = mp.Pool(n)
        if option_type == OptionType.f:
            choices = [[option] for option in options_dict[compiler_name]]
        elif option_type == OptionType.fno:
            choices = [["-fno-" + option[2:]] for option in options_dict[compiler_name]]
        elif option_type == OptionType.main:
            main_opts = ["-O0", "-O1", "-O2", "-O3"]
            main_opts.remove(default_option)
            choices = [[opt] for opt in main_opts]
        else:
            choices = []
        # choices.extend(list(combinations(options_dict[compiler_name], 2)))
        choices_list = [[] for i in range(n)]
        for i, choice in enumerate(choices):
            choices_list[i % n].append(choice)
        
        for i in range(n):
            res_list.append(pool.apply_async(select_trigger_options, (compiler_name, oldinfo, choices_list[i], i)))
        
        pool.close()
        pool.join()

        result[compiler_name] = []
        for res in res_list:
            result[compiler_name].extend(res.get())

    for compiler_name in result:
        if len(result[compiler_name]) == 0:
            continue
        print("{} can trigger with".format(compiler_name))
        for opt in result[compiler_name]:
            print(opt)

    return result


if __name__ == "__main__":
    caseid = int(sys.argv[1])
    bug = Bug(caseid)
    optionmgr = OptionMgr.get_optionmgr(sys.argv[2])
    if len(sys.argv) > 3:
        default_maxlength_comb = int(sys.argv[3])
    if len(sys.argv) > 4:
        default_heuristic_limit = int(sys.argv[4])
    optionmgr.select_gcc(bug, 24)