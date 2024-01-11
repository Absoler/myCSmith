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
import itertools
from compilerbugs import Bug, Compiler, CompilerType, pintool
import json
import re
sys.path.append("{}/test".format(root_dir))
import compare
import multiprocessing as mp
import option


regression_files = os.listdir("{}/regression/result".format(root_dir))
gcc_dirs = [ "{}/regression/result/{}".format(root_dir, gcc_dir) for gcc_dir in regression_files if "gcc" in gcc_dir ]
clang_dirs = [ "{}/regression/result/{}".format(root_dir, clang_dir) for clang_dir in regression_files if "clang" in clang_dir ]

default_bugs_path = "{}/regression/bugs.json".format(root_dir)
default_merge_path = "{}/regression/mergebugs.json".format(root_dir)
visit_compile_names = set() # record checked compilers, avoid redundant check



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



''' this function is used to test more compilers on existed bugs,
    and extend triggered bug list
'''
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
            ret = pintool("./a.out")
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

''' similar to `test_docker`
'''
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
            
            ret = pintool("./a.out")
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
    cp_bugs_map_gcc = {}
    cp_bugs_map_clang = {}
    bug_cps_map_both = {}
    for bug in bugs_map:
        cpHash = bug.compilerHash
        if bug.compilerType == CompilerType.gcc:
            if cpHash not in cp_bugs_map_gcc:
                cp_bugs_map_gcc[cpHash] = []
            cp_bugs_map_gcc[cpHash].append(bug)
        elif bug.compilerType == CompilerType.clang:
            if cpHash not in cp_bugs_map_clang:
                cp_bugs_map_clang[cpHash] = []
            cp_bugs_map_clang[cpHash].append(bug)
        elif bug.compilerType == CompilerType.both:
            bug_cps_map_both[bug] = bugs_map[bug]
    
    regression_cnt = [0, 0]
    constraints = [
        "regression",
        "newest",
    ]
    constraint_str = "##constraints: "
    for i in range(len(constraints)):
        if option & (1<<i):
            constraint_str += constraints[i] + " "
    print(constraint_str)
    for gccs in list(cp_bugs_map_gcc.keys()):
        if option & 1 != 0 and not Compiler.isregression(gccs):
            cp_bugs_map_gcc.pop(gccs)
            continue
        if option & 2 != 0 and not Compiler.hasnewest(gccs, CompilerType.gcc):
            cp_bugs_map_gcc.pop(gccs)
            continue
        gccs_names = Compiler.uncompress(gccs, CompilerType.gcc)
        print("{} bugs for {} {:010b}".format(len(cp_bugs_map_gcc[gccs]), gccs_names, gccs))
        print("case ids {}".format([bug.id for bug in cp_bugs_map_gcc[gccs]]))
        regression_cnt[0] += 1
    for clangs in cp_bugs_map_clang:
        if option & 1 != 0 and not Compiler.isregression(clangs):
            continue
        if option & 2 != 0 and not Compiler.hasnewest(clangs, CompilerType.clang):
            continue
        clangs_names = Compiler.uncompress(clangs, CompilerType.clang)
        print("{} bugs for {} {:016b}".format(len(cp_bugs_map_clang[clangs]), clangs_names, clangs))
        print("case ids {}".format([bug.id for bug in cp_bugs_map_clang[clangs]]))
        regression_cnt[1] += 1
    
    print("## notice! these bugs are triggered both by gcc and clang")
    for bug in bug_cps_map_both:
        print("case id {} in {}".format(bug.id, bug_cps_map_both[bug]))
    print("regression sum {}".format(regression_cnt))

    return (cp_bugs_map_gcc, cp_bugs_map_clang)


def validateClang(bugs_map:dict):
    tempdir = "{}/regression/validateclang".format(root_dir)
    if not os.path.exists(tempdir):
        os.mkdir(tempdir)
    
    os.chdir(tempdir)
    ''' {
            "compiler_name" : [hit_count, all_count, [miss_bug_ids]]
        }
    '''
    res = {}
    for bug in bugs_map:
        compiler_names = bugs_map[bug]
        if bug.compilerType != "clang":
            continue
        os.system("cp {}/regression/caserepo/output{}.c ./output.c".format(root_dir, bug.id))
        for compiler_name in compiler_names:
            ret = os.system("which {}".format(compiler_name))
            if ret != 0:
                continue
            
            if compiler_name not in res:
                res[compiler_name] = [0, 0, []]
            
            res[compiler_name][1] += 1

            version = Compiler.get(compiler_name).version
            os.system("{} output.c {} -o elf.clang >/dev/null 2>&1".format(compiler_name, opt_option))
            
            optionmgr = option.OptionMgr.get_optionmgr(compiler_name)
            optionmgr.checkvalidation()
            optlist = optionmgr.getvalidseq()
            options_str = option.optiter_to_str(optlist)
            os.system("clang-{} output.c -S -Xclang -disable-llvm-passes -emit-llvm -o output.ll >/dev/null 2>&1".format(version))
            os.system("sed '/[aA]ttr/s/optnone//g' output.ll -i")
            os.system("opt-{} output.ll {} -o output.bc >/dev/null 2>&1".format(version, options_str))
            os.system("llc-{} output.bc -filetype=obj -o output.o".format(version))
            os.system("clang-{} output.o -o elf.opt >/dev/null 2>&1".format(version))

            pintool("./elf.clang")
            os.system("cp descript.out descript.clang")

            pintool("./elf.opt")
            os.system("cp descript.out descript.opt")

            clang_info = compare.parse("./descript.clang")
            opt_info = compare.parse("./descript.opt")
            if clang_info == opt_info:
                res[compiler_name][0] += 1
            else:
                res[compiler_name][2].append(bug.id)

    for compiler_name in res:
        print(compiler_name)
        print(res[compiler_name])

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="regression bug analysis tool")
    parser.add_argument("--init", action="store_true", help="init bugs json from ./result directory")
    parser.add_argument("--testlocal", "-tl", default="", help="specfiy a file recorded local compilers to be tested")
    parser.add_argument("--testdocker", "-td", default="", help="specfiy a file recorded compiler docker image names")
    parser.add_argument("--merge", "-m", nargs="+", default="", help="merge the provided bug files and dump it as `mergebugs.json`")
    parser.add_argument("--nproc", "-n", default=1, type=int, help="allow n jobs at once")
    parser.add_argument("--output", "-o", default="", help="specfiy dump path")
    parser.add_argument("--input", "-i", default="", help="specify the input bugs file")


    parser.add_argument("--analyzegroup", "-ag", default=-1, type=int, help="group bugs by compiler hash, specify constraints by bit num, `0x1` is regression, `0x2` cares newest version")
    parser.add_argument("--validateClang", "-vC", action="store_true", help="check whether `clang` and `opt` trigger the same optimization bug")
    parser.add_argument("--reducegcc", "-rg", action="store_true", help="find the minimum option combination that blocks gcc bug")
    parser.add_argument("--reduceclang", "-rc", action="store_true", help="find the minimum option combination that blocks clang bug")
    args = parser.parse_args()

    if args.init:
        init_bugs()
        exit(0)

    outputpath = args.output
    nproc = int(args.nproc)
    
    if args.merge:
        paths = args.merge
        bugs_map_list = [load_bugs(path) for path in paths]
        res = merge_bugs(bugs_map_list)
        save_bugs(res, outputpath)
        exit(0)
    
    bugs_map = load_bugs(args.input)
    print("## {} bugs in all".format(len(bugs_map)))
    Compiler.init_bugs(bugs_map)

    if args.validateClang:
        validateClang(bugs_map)
        exit(0)
    
    if args.analyzegroup != -1:
        cp_bugs_map_gcc, cp_bugs_map_clang = analyze_group(bugs_map, args.analyzegroup)
    
    if args.reducegcc:
        bugs = list(itertools.chain(*cp_bugs_map_gcc.values()))
        newest_cp = Compiler.gccs[-1]
        optionmgr = option.OptionMgr.get_optionmgr(newest_cp.name)
        for bug in bugs:
            optionmgr.select_gcc(bug, nproc)
        exit(0)


    if args.testlocal:
        results_queue = mp.Queue()

        if os.path.exists("{}/regression/{}".format(root_dir, args.testlocal)):
            localfile = open("{}/regression/{}".format(root_dir, args.testlocal), "r")
        else:
            localfile = open(args.testlocal, "r")
        compiler_names = localfile.readlines()
        compiler_names = [name.strip() for name in compiler_names]
        localfile.close()

        nproc = min(nproc, len(compiler_names))
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