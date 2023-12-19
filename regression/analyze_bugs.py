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
import multiprocessing as mp



regression_files = os.listdir("{}/regression/result".format(root_dir))
gcc_dirs = [ "{}/regression/result/{}".format(root_dir, gcc_dir) for gcc_dir in regression_files if "gcc" in gcc_dir ]
clang_dirs = [ "{}/regression/result/{}".format(root_dir, clang_dir) for clang_dir in regression_files if "clang" in clang_dir ]

default_bugs_path = "{}/regression/bugs.json".format(root_dir)
default_merge_path = "{}/regression/mergebugs.json".format(root_dir)
visit_compile_names = set() # record checked compilers, avoid redundant check

class Bug:
    def __init__(self, id:int, info = None) -> None:
        self.id = id
        self.info = info

        # filled by Compiler.init_bugs()
        self.compilerPrefix = ""
        self.compilerHash = 0
    
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
def load_bugs(bugs_path = default_bugs_path):
    if not bugs_path:
        bugs_path = default_bugs_path
    bugs_file = open(bugs_path, "r")
    bugs_json = json.load(bugs_file)
    bugs_map = {}
    for bug_json in bugs_json:
        bug = Bug.fromdict(bug_json)
        rela_compilers = bug_json["rela_compilers"]
        bugs_map[bug] = rela_compilers
    return bugs_map


def merge_bugs(bugs_map_list:list):
    if len(bugs_map_list) == 1:
        return bugs_map_list[0]
    
    bugs_ret = bugs_map_list[1]
    
    # check, may be unnecessary
    bugs = set(bugs_ret.keys())
    for bugs_map in bugs_map_list[1:]:
        assert(set(bugs_map.keys()) == bugs)
    
    for bugs_map in bugs_map_list[1:]:
        for bug in bugs:
            for cp in bugs_map[bug]:    # cp is compiler
                if cp not in bugs_ret[bug]:
                    bugs_ret[bug].append(cp)
    return bugs_ret


def init_bugs():
    bugs_map = {}
    descript_re = re.compile(r"(gcc|clang)\-([\d\.]+)_(\d+)descript.out")
    if os.path.exists(default_bugs_path):
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

def save_bugs(bugs_map, save_path = default_bugs_path):
    if not save_path:
        save_path = default_bugs_path
    bugs_file = open(save_path, "w")
    bugs_json = []
    for bug in bugs_map:
        bug_json = dict(bug)
        bug_json["rela_compilers"] = bugs_map[bug]
        bugs_json.append(bug_json)
    json.dump(bugs_json, bugs_file, indent=4)
    bugs_file.close()



class Compiler:
    compiler_map = {}
    gccs = []
    clangs = []
    
    @classmethod
    def add(cls, compiler_name:str):
        if compiler_name not in Compiler.compiler_map:
            Compiler.compiler_map[compiler_name] = Compiler(*compiler_name.split("-"))
        
    @classmethod
    def get(cls, compiler_name:str):
        Compiler.add(compiler_name)
        return Compiler.compiler_map[compiler_name]
    
    @classmethod
    def init_bugs(cls, bugs_map):
        for bug in bugs_map:
            for compiler_name in bugs_map[bug]:
                compiler = Compiler.get(compiler_name)
                compiler.bugs.append(bug)
        compilers = list(cls.compiler_map.values())
        cls.gccs = [compiler for compiler in compilers if compiler.prefix == "gcc"]
        cls.gccs.sort()
        cls.clangs = [compiler for compiler in compilers if compiler.prefix == "clang"]
        cls.clangs.sort()
        for bug in bugs_map:
            compilerHash = 0
            for compiler_name in bugs_map[bug]:
                compiler = Compiler.get(compiler_name)
                if compiler.prefix == "gcc":
                    id = cls.gccs.index(compiler)
                elif compiler.prefix == "clang":
                    id = cls.clangs.index(compiler)
                compilerHash += (1<<id)
                if not bug.compilerPrefix:
                    bug.compilerPrefix = "gcc" if compiler.prefix == "gcc" else "clang"
            bug.compilerHash = compilerHash

    @classmethod
    def uncompress(cls, cpHash:int, prefix:str):
        res = []
        cps = cls.gccs if prefix == "gcc" else cls.clangs
        for i in range(len(cps)):
            if cpHash & (1<<i) != 0:
                res.append(cps[i].name)
        return res
    
    @classmethod
    def isregression(cls, cpHash) -> bool:
        i = 1
        state = 0
        while i < cpHash:
            if i & cpHash != 0:
                if state & 1 == 0:
                    state += 1
            else:
                if state & 1 == 1:
                    state += 1
            i *= 2
        return state > 2
    
    @classmethod
    def hasnewest(cls, cpHash:int, prefix:str) -> bool:
        if prefix == "gcc":
            return (1 << (len(Compiler.gccs) - 1)) & cpHash != 0
        elif prefix == "clang":
            return (1 << (len(Compiler.clangs) - 1)) & cpHash != 0
        return False
    
    def __init__(self, prefix:str, version:str) -> None:
        self.prefix = prefix
        self.version = version

        ''' assume the version code consists of at most 3 2-digit number
        '''
        self.varr = list(map(int, version.split(".")))
        if len(self.varr) < 3:
            self.varr.extend([0] * (3 - len(self.varr)))
        self.vnum = self.varr[0] * 10**4 + self.varr[1]  * 10**2 + self.varr[2]

        self.bugs = []
    

    def __lt__(self, other):
        return self.vnum < other.vnum

    def __eq__(self, other):
        return self.prefix == other.prefix and self.vnum == other.vnum
    
    def ismain(self) -> bool:
        if self.prefix == "clang":
            return self.vnum[1] == 0 and self.vnum[2] == 0
        elif self.prefix == "gcc":
            return self.vnum[1] == 2 and self.vnum[2] == 0
        else:
            return False
    
    @property
    def name(self):
        return self.prefix + "-" + self.version

def test_docker(image_names:list, bugs_map):
    
    tempdir = "{}/regression/tempdir".format(root_dir)
    os.system("mkdir {} -p".format(tempdir))
    os.chdir(tempdir)

    source_dir = ""
    if os.path.exists("{}/regression/docker.volume".format(root_dir)):
        mountinfo = open("{}/regression/docker.volume".format(root_dir), "r").readline().strip()
        hostdir, containerdir = mountinfo.split(":")
        if os.path.samefile(os.path.commonprefix([containerdir, root_dir]), containerdir):
            source_dir = os.path.join(hostdir, os.path.relpath(tempdir, containerdir))
    if source_dir == "":
        print("ERROR: no valid test path, should be accessible from the host")
        return

    for compiler_image in image_names:
        ''' check whether the image exists
        '''
        res = os.popen("docker image ls '{}' | wc -l".format(compiler_image)).read().strip()
        if res != "2":
            print("WARN: no docker image {}".format(compiler_image))
            continue
        
        print("start testing docker image {}".format(compiler_image))
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
                print("WARN: {} fail to compile {} with docker image".format(compiler_image, casepath))
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
    return bugs_map


def test_local(pnum:int, compiler_names, inputpath:str, queue):

    tempdir = "{}/regression/tempdir/{}".format(root_dir, pnum)
    os.system("mkdir {} -p".format(tempdir))
    os.chdir(tempdir)

    bugs_map = load_bugs(inputpath)
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
        queue.put(bugs_map)

def analyze_group(bugs_map, option):
    gccs_bugs_map = {}
    clangs_bug_map = {}
    for bug in bugs_map:
        cpHash = bug.compilerHash
        if bug.compilerPrefix == "gcc":
            if cpHash not in gccs_bugs_map:
                gccs_bugs_map[cpHash] = []
            gccs_bugs_map[cpHash].append(bug)
        elif bug.compilerPrefix == "clang":
            if cpHash not in clangs_bug_map:
                clangs_bug_map[cpHash] = []
            clangs_bug_map[cpHash].append(bug)

    regression_cnt = [0, 0]
    constraints = [
        "regression",
        "newest",
    ]
    constraint_str = "##constraints: "
    for i in range(len(constraints)):
        if option & (1<<i):
            constraint_str += constraints[i] + " "
    for gccs in gccs_bugs_map:
        if option & 1 != 0 and not Compiler.isregression(gccs):
            continue
        if option & 2 != 0 and not Compiler.hasnewest(gccs, "gcc"):
            continue
        gccs_names = Compiler.uncompress(gccs, "gcc")
        print("{} bugs for {} {:020b}".format(len(gccs_bugs_map[gccs]), gccs_names, gccs))
        print("case ids {}".format([bug.id for bug in gccs_bugs_map[gccs]]))
        regression_cnt[0] += 1
    for clangs in clangs_bug_map:
        if option & 1 != 0 and not Compiler.isregression(clangs):
            continue
        if option & 2 != 0 and not Compiler.hasnewest(clangs, "clang"):
            continue
        clangs_names = Compiler.uncompress(clangs, "clang")
        print("{} bugs for {} {:020b}".format(len(clangs_bug_map[clangs]), clangs_names, clangs))
        print("case ids {}".format([bug.id for bug in clangs_bug_map[clangs]]))
        regression_cnt[1] += 1
    print("regression sum {}".format(regression_cnt))

    return (gccs_bugs_map, clangs_bug_map)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="regression bug analysis tool")
    parser.add_argument("--init", action="store_true")
    parser.add_argument("--testlocal", "-tl", default="", help="specfiy a file recorded local compilers to be tested")
    parser.add_argument("--testdocker", "-td", default="", help="specfiy a file recorded compiler docker image names")
    parser.add_argument("--merge", "-m", nargs="+", default="", help="merge the provided bug files and dump it as `mergebugs.json`")
    parser.add_argument("--nproc", "-n", default=1, type=int, help="allow n jobs at once")
    parser.add_argument("--output", "-o", default="", help="specfiy dump path")
    parser.add_argument("--input", "-i", default="", help="specify the input bugs file")


    parser.add_argument("--analyzegroup", "-ag", default=-1, type=int, help="group bugs by compiler hash, specify constraints by bit num, `0x1` is regression, `0x2` cares newest version")
    
    args = parser.parse_args()

    if args.init:
        init_bugs()
        exit(0)

    outputpath = args.output
    
    if args.merge:
        paths = args.merge
        bugs_map_list = [load_bugs(path) for path in paths]
        res = merge_bugs(bugs_map_list)
        save_bugs(res, outputpath)
        exit(0)
    
    bugs_map = load_bugs(args.input)
    print("## {} bugs in all".format(len(bugs_map)))
    Compiler.init_bugs(bugs_map)
    
    if args.analyzegroup != -1:
        analyze_group(bugs_map, args.analyzegroup)
    
    if args.testlocal:
        results_queue = mp.Queue()

        if os.path.exists("{}/regression/{}".format(root_dir, args.testlocal)):
            localfile = open("{}/regression/{}".format(root_dir, args.testlocal), "r")
        else:
            localfile = open(args.testlocal, "r")
        compiler_names = localfile.readlines()
        compiler_names = [name.strip() for name in compiler_names]
        localfile.close()

        nproc = args.nproc
        compiler_names_arr = [[] for i in range(nproc)]
        for i, name in enumerate(compiler_names):
            compiler_names_arr[i%nproc].append(name)

        jobs = []
        for i in range(nproc):
            p = mp.Process(target=test_local, args=(i, compiler_names_arr[i], args.input, results_queue, ))
            jobs.append(p)
            p.start()
        for i in range(nproc):
            jobs[i].join()
        
        results = [results_queue.get() for i in range(nproc)]
        local_res = merge_bugs(results)
        bugs_map = merge_bugs([bugs_map, local_res])

    save_bugs(bugs_map, outputpath)
    

    if args.testdocker and os.system("which docker") == 0:
        results_queue = mp.Queue()

        if os.path.exists("{}/regression/{}".format(root_dir, args.testdocker)):
            imagefile = open("{}/regression/{}".format(root_dir, args.testdocker), "r")
        else:
            imagefile = open(args.testdocker, "r")

        image_names = imagefile.readlines()
        image_names = [name.strip() for name in image_names]
        imagefile.close()

        docker_res = test_docker(image_names, bugs_map)
        
        bugs_map = merge_bugs([bugs_map, docker_res])

        
    save_bugs(bugs_map, outputpath)