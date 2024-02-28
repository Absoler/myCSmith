#!/usr/bin/python3

root_dir=
default_option=

import sys, re, json, os
from enum import Enum
sys.path.append("{}/regression".format(root_dir))
sys.path.append("{}/test".format(root_dir))
from compilerbugs import pintool, Bug, compile
import compare

regression_files = os.listdir("{}/regression/result".format(root_dir))
gcc_dirs = [ "{}/regression/result/{}".format(root_dir, gcc_dir) for gcc_dir in regression_files if "gcc" in gcc_dir ]
clang_dirs = [ "{}/regression/result/{}".format(root_dir, clang_dir) for clang_dir in regression_files if "clang" in clang_dir ]

default_bugs_path = "{}/regression/bugs.json".format(root_dir)
default_bugs_out_path = "{}/regression/bugs.out.json".format(root_dir)
default_merge_path = "{}/regression/mergebugs.json".format(root_dir)


########################################
#                                      #
#    process bug.json                  #
#                                      #
########################################

''' Args:    implicit the file "./bugs.json"
    Return:  a dict, mapping `Bug` object to its relative compilers names (a list)
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

def save_bugs(bugs_map, save_path = default_bugs_out_path):
    if not save_path:
        save_path = default_bugs_out_path
    bugs_file = open(save_path, "w")
    bugs_json = []
    for bug in bugs_map:
        bug_json = dict(bug)
        bug_json["rela_compilers"] = bugs_map[bug]
        bugs_json.append(bug_json)
    json.dump(bugs_json, bugs_file, indent=4)
    bugs_file.close()

########################################
#                                      #
#    process final data json           #
#                                      #
########################################

''' check info's correctness
'''

def validate(path:str):
    count = [0, 0, 0, 0, 0, 0]
    with open(path, "r") as final_f:
        finaljson = json.load(final_f)
    
    workdir = "{}/regression/data".format(root_dir)
    os.system("mkdir -p {}".format(workdir))
    os.chdir(workdir)
    for bug in finaljson:
        count[0] += 1
        bug_fail = False
        num = int(bug['id'])
        oldinfo = compare.Info.fromdict(bug['info'])
        os.system("cp {}/regression/caserepo/output{}.c output.c".format(root_dir, num))
        cp_d = bug['conditions']
        for cp in cp_d:
            if os.system("which {} >/dev/null".format(cp)):
                continue
            count[2] += 1
            cp_fail = False
            
            if cp_d[cp]["default"]:
                count[4] += 1
                ret = compile(cp, "")
                if ret != 0:
                    cp_fail = True
                    print("{} compile fail".format(num))
                    continue
                pintool()
                info = compare.parse()
                if info != oldinfo:
                    cp_fail = True
                    print("{} not match".format(num))
                else:
                    count[5] += 1
            else:
                for opt in cp_d[cp]["extra_options"]:
                    count[4] += 1
                    ret = compile(cp, opt)
                    if ret != 0:
                        cp_fail = True
                        print("{} compile fail".format(num))
                        continue
                    pintool()
                    info = compare.parse()
                    if info != oldinfo:
                        cp_fail = True
                        print("{} not match".format(num))
                    else:
                        count[5] += 1
            if not cp_fail:
                count[3] += 1
            else:
                bug_fail = True
        if not bug_fail:
            count[1] += 1
    print(count)


########################################
#                                      #
#    process migration test output     #
#                                      #
########################################

''' { num: { compiler: [opts]}}

'''
def load_migration_output(path:str):
    with open(path, "r") as migration_output_f:
        lines = migration_output_f.readlines()
    
    lines = [line.strip() for line in lines]

    num = -1
    cp = ""
    opt = ""
    res = {}
    for line in lines:
        if line.startswith('...'):
            num = int(line.split()[2])
        elif line.endswith('can trigger with'):
            cp = line.split()[0]
        elif line.startswith('-'):
            opt = line
            if num not in res:
                res[num] = {}
            if cp not in res[num]:
                res[num][cp] = []

            res[num][cp].append(opt)
    
    return res

def merge_migration_to_final(migration_d:dict, finaljson:list):
    workdir = "{}/regression/data2".format(root_dir)
    os.system("mkdir -p {}".format(workdir))
    os.chdir(workdir)
    
    for bug in finaljson:
        num = int(bug['id'])
        if num not in migration_d:
            continue
        
        oldinfo = compare.Info.fromdict(bug['info'])
        cp_d = migration_d[num]
        os.system("cp {}/regression/caserepo/output{}.c output.c".format(root_dir, num))

        for cp in cp_d:
            opts = cp_d[cp]
            ret = compile(cp, opts[0])
            if ret != 0:
                print("{} fail to compile".format(num))
                assert(0)

            pintool()
            info = compare.parse()
            if info == oldinfo:
                if cp not in bug["conditions"]:
                    bug["conditions"][cp] = {
                        "default": False,
                        "extra_options": []
                    }
                
                for opt in opts:
                    if opt not in bug["conditions"][cp]["extra_options"]:
                        bug["conditions"][cp]["extra_options"].append(opt)
    
    return finaljson
