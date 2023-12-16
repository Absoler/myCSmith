#!/usr/bin/python3

opt_option=
test_type=
pin_root=
root_dir=

'''

This script is responsible for regression test of compilers with different versions.

### bug info
    a bug is identified with its `case id` and `descript info`
    bugs'info is cached in ./bugs.json, and can be extended,
    it inits with main version compilers' bug info, which located in ./result
    and in the later (distributed maybe) analysis, we need to figure out which versions relates to the bugs

Functionality:
    1. test compilers, regression test for one bug, one compiler or all
    2. analyze bug info and answer the query or generate statistical chart


Compiler Resources:
    1. docker image
    2. built, `PATH` has been set properly
    constraint: tag name == compiler suffix

'''

import os, sys, argparse
import json
import re
sys.path.append("{}/test".format(root_dir))
import compare

if __name__ != "__main__":
    raise ImportError("this file can't act as a library")
if os.getcwd() != "{}/regression".format(root_dir):
    os.chdir("{}/regression".format(root_dir))


regression_files = os.listdir("{}/regression/result".format(root_dir))
gcc_dirs = [ "{}/regression/result/{}".format(root_dir, gcc_dir) for gcc_dir in regression_files if "gcc" in gcc_dir ]
clang_dirs = [ "{}/regression/result/{}".format(root_dir, clang_dir) for clang_dir in regression_files if "clang" in clang_dir ]

bugs_path = "{}/regression/bugs.json".format(root_dir)


class Bug:
    def __init__(self, id:int, info = None) -> None:
        self.id = id
        self.info = info
    
    def __eq__(self, bug) -> bool:
        return self.id == bug.id and self.info == bug.info
    
    def __hash__(self) -> int:
        return hash(self.id) + hash(self.info)
    
    def keys(self):
        return ("id", "info", )
    
    def __getitem__(self, key):
        if key == "info":
            return dict(self.info)
        else:
            return getattr(self, key)

    @classmethod
    def fromdict(cls, data):
        return Bug(data["id"], compare.Info.fromdict(data["info"]))


''' 
    Args:
        implicit the file "./bugs.json"
    
    Return:
        a dict, mapping `Bug` object to its relative compilers names (a list)

'''
def load_bugs():
    bugs_file = open(bugs_path, "r")
    bugs_json = json.load(bugs_file)
    bugs_map = {}
    for bug_json in bugs_json:
        bug = Bug.fromdict(bug_json)
        rela_compilers = bug_json["rela_compilers"]
        bugs_map[bug] = rela_compilers
    return bugs_map


def init_bugs():
    bugs_map = {}
    descript_re = re.compile(r"(gcc|clang)\-([\d\.]+)_(\d+)descript.out")
    if os.path.exists(bugs_path):
        print("WARN: bugs file existed, no need for init")
        return load_bugs()
    
    compiler_dirs = gcc_dirs + clang_dirs
    for compiler_dir in compiler_dirs:
        compiler_files = os.listdir(compiler_dir)
        for file in compiler_files:
            if file.endswith("descript.out"):
                matchObj = descript_re.match(file)
                compiler, caseid = matchObj.group(1) + "-" + matchObj.group(2), int(matchObj.group(3))
                bug = Bug(caseid, info = compare.parse("{}/{}".format(compiler_dir, file)))
                if bug in bugs_map:
                    bugs_map[bug].append(compiler)
                else:
                    bugs_map[bug] = [compiler]
    
    save_bugs(bugs_map)
    print("init {} bugs in total".format(len(bugs_map)))
    return bugs_map

def save_bugs(bugs_map):
    bugs_file = open(bugs_path, "w")
    bugs_json = []
    for bug in bugs_map:
        bug_json = dict(bug)
        bug_json["rela_compilers"] = bugs_map[bug]
        bugs_json.append(bug_json)
    json.dump(bugs_json, bugs_file, indent=4)
    bugs_file.close()



class Compiler:
    compiler_map = {}
    
    @classmethod
    def add(cls, compiler_name:str):
        if compiler_name not in Compiler.compiler_map:
            Compiler.compiler_map[compiler_name] = Compiler(*compiler_name.split("-"))
        
    @classmethod
    def get(cls, compiler_name:str):
        Compiler.add(compiler_name)
        return Compiler.compiler_map[compiler_name]

    def __init__(self, name:str, version:str) -> None:
        self.name = name
        self.version = version

        self.bugs = []



if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="regression bug analysis tool")
    parser.add_argument("--init", "-i", action="store_true")
    parser.add_argument("--analyzelocal", "-al", default="", help="specfiy a file recorded local compilers to be tested")
    parser.add_argument("--analyzedocker", "-ad", default="", help="specfiy a file recorded compiler docker image names")
    
    args = parser.parse_args()

    if args.init:
        init_bugs()
        exit(0)
    
    visit_compile_names = set() # record checked compilers, avoid redundant check
    bugs_map = load_bugs()
    print("## {} bugs in all".format(len(bugs_map)))
    for bug in bugs_map:
        for compiler_name in bugs_map[bug]:
            compiler = Compiler.get(compiler_name)
            compiler.bugs.append(bug)
    

    tempdir = "{}/regression/tempdir".format(root_dir)
    os.system("mkdir {} -p".format(tempdir))
    os.chdir(tempdir)

    if args.analyzelocal:
        if os.path.exists("{}/regression/{}".format(root_dir, args.analyzelocal)):
            localfile = open("{}/regression/{}".format(root_dir, args.analyzelocal), "r")
        else:
            localfile = open(args.analyzelocal, "r")
        compiler_names = localfile.readlines()
        compiler_names = [name.strip() for name in compiler_names]
        localfile.close()
        for compiler_name in compiler_names:
            ''' check compiler whether configured in current environment
            '''
            ret = os.system("which {}".format(compiler_name))
            if ret != 0:
                print("WARN: {} not configured".format(compiler_name))
                continue
            
            if compiler_name in visit_compile_names:
                print("WARN: repeat check {}".format(compiler_name))
                continue
            visit_compile_names.add(compiler_name)
            compiler = Compiler.get(compiler_name)
            for bug in bugs_map:
                caseid = bug.id
                casepath = "{}/regression/caserepo/output{}.c".format(root_dir, caseid)
                os.system("cp {} output{}.c".format(casepath, caseid))
                ret = os.system("{} output{}.c {} -g 2>/dev/null".format(compiler_name, caseid, opt_option))
                if ret != 0:
                    print("WARN: {} fail to compile {}".format(compiler, casepath))
                    continue
                
                ret = os.system("timeout -s SIGTERM 5s {}/pin -t {}/checkAccess/obj-intel64/checkAccess.so -- ./a.out {} func ./ 1>/dev/null".format(pin_root, root_dir, test_type))
                res = os.popen("cat result.out").read().strip()
                if ret == 0 and res == "1":
                    info = compare.parse("./descript.out")
                    if info == bug.info and compiler_name not in bugs_map[bug]:
                        ''' find new compiler which also trigger the bug
                        '''
                        bugs_map[bug].append(compiler_name)
                        compiler.bugs.append(bug)
                        print("--> found bug: {} on {}".format(caseid, compiler_name))

    save_bugs(bugs_map)
    

    if args.analyzedocker and os.system("which docker") == 0:
        if os.path.exists("{}/regression/{}".format(root_dir, args.analyzedocker)):
            imagefile = open("{}/regression/{}".format(root_dir, args.analyzedocker), "r")
        else:
            imagefile = open(args.analyzedocker, "r")

        source_dir = ""
        if os.path.exists("{}/regression/docker.volume".format(root_dir)):
            mountinfo = open("{}/regression/docker.volume".format(root_dir), "r").readline().strip()
            hostdir, containerdir = mountinfo.split(":")
            if os.path.samefile(os.path.commonprefix([containerdir, root_dir]), containerdir):
                source_dir = os.path.join(hostdir, os.path.relpath(tempdir, containerdir))
        if source_dir == "":
            print("ERROR: no valid test path, should be accessible from the host")
            exit(1)

        
        image_names = imagefile.readlines()
        image_names = [name.strip() for name in image_names]
        imagefile.close()

        for compiler_image in image_names:
            ''' check whether the image exists
            '''
            res = os.popen("docker image ls '{}' | wc -l".format(compiler_image)).read().strip()
            if res != "2":
                print("WARN: no docker image {}".format(compiler_image))
                continue
            
            compiler_name = compiler_image.replace(":", "-")
            compiler_prefix = compiler_image.split(":")[0]
            if compiler_name in visit_compile_names:
                print("WARN: repeat check {}".format(compiler_name))
                continue
            visit_compile_names.add(compiler_name)
            compiler = Compiler.get(compiler_name)
            for bug in bugs_map:
                caseid = bug.id
                casepath = "{}/regression/caserepo/output{}.c".format(root_dir, caseid)
                os.system("cp {} output.c".format(casepath))
                ret = os.system("docker run --rm -v {}:/root -w /root {} {} /root/output.c {} 2>/dev/null".format(source_dir, compiler_image, compiler_prefix, opt_option))
                if ret != 0:
                    print("WARN: {} fail to compile {} with docker image".format(compiler, casepath))
                    continue
                
                # same as local test
                ret = os.system("timeout -s SIGTERM 5s {}/pin -t {}/checkAccess/obj-intel64/checkAccess.so -- ./a.out {} func ./ 1>/dev/null".format(pin_root, root_dir, test_type))
                res = os.popen("cat result.out").read().strip()
                if ret == 0 and res == "1":
                    info = compare.parse("./descript.out")
                    if info == bug.info and compiler_name not in bugs_map[bug]:
                        ''' find new compiler which also trigger the bug
                        '''
                        bugs_map[bug].append(compiler_name)
                        compiler.bugs.append(bug)
                        print("--> found bug: {} on {}".format(caseid, compiler_name))

    save_bugs(bugs_map)