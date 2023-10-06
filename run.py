#!/usr/local/bin/python3

# ------------------------------
#
#   this script invoke csmith to generate C files and judge
#   whether a multi-load problem happen on a compiler
# 
#   args[1]: number of tests
#   args[2]: core number
#
#   python version of run.kb, support multi-process
# ------------------------------

import sys, os, multiprocessing

opt_option=
test_type=
pin_root=


''' init part
'''
args = sys.argv
if len(args) < 1:
    limit = 10000000
else:
    limit = int(args[1])

if len(args) < 2:
    concurrent = 1
else:
    concurrent = int(args[2])

if test_type == "0":
    type_option = ""
else:
    type_option = "--store"

compilers = ("gcc-12.1", "clang")

# 1th line of base.out is the number of C cases, 2th line is the next of current reduced case
with open("base.out", "r") as base_file:
    base = int(base_file.readline())

''' process task
'''
def run_csmith(id:int, mod:int, base_lock):
    case_id = id
    run_err=0

    cache_prefix:str = "cache/" + str(id) + "/"
    if not os.path.exists(cache_prefix):
        os.makedirs(cache_prefix)

    while case_id < limit:
        os.system(f"build/src/csmith {type_option} --copyPropagation --no-safe-math --no-bitfields --no-volatiles --probability-configuration ./prob.txt  -o runtime/output{id}.c 1>/dev/null")
    
        cur_cnt = -1
        for compiler in compilers:
            ret = os.system(f"{compiler} runtime/output{id}.c -gdwarf-4 -w {opt_option} -o runtime/output{id}")
            if ret > 0:
                print(f"{compiler} compilation error")
                os.system(f"cp runtime/output{id}.c compileFail/{case_id}output{id}.c")
                break
            
            ret = os.system(f"timeout -s SIGTERM 5s {pin_root}/pin -t checkAccess/obj-intel64/checkAccess.so -- runtime/output{id} {test_type} func {cache_prefix} 1>/dev/null")
            if ret == 134:
                os.system(f"cp runtime/output{id}.c pinErr/{case_id}output{id}.c")
            
            if ret > 0:
                run_err += 1
                continue
        
            with open(f"{cache_prefix}/result.out", "r") as result_file:
                result = int(result_file.readline())
            if result == 1:
                

                if cur_cnt == -1:
                    base_lock.acquire()
                    with open("base.out", "r") as base_file:
                        cur_cnt = int(base_file.readline())
                    os.system(f"./record.kb {compiler} {cur_cnt} {cache_prefix} {id}")
                    os.system(f"sed \"1c {cur_cnt+1}\" -i base.out")
                    base_lock.release()
                else:
                    os.system(f"./record.kb {compiler} {cur_cnt} {cache_prefix} {id}")

                print(f"{compiler} fail at {cur_cnt}")
        
        

        if case_id % 1000 == id:
            os.system(f"echo `date`")
            print(f"[process {id}]: {case_id} cases tested, runtime error: {run_err}")
        
        case_id += mod
    
    os.system(f"echo `date`")
    print(f"[process {id}]: {case_id} cases tested, runtime error: {run_err}")



''' main process
'''
def main(num:int = 1):
    base_lock = multiprocessing.Manager().Lock()

    processes:list = []
    for i in range(num):
        processes.append(multiprocessing.Process(target=run_csmith, args=(i, num, base_lock)))
        processes[i].start()
    
    for i in range(num):
        processes[i].join()
    



main(concurrent)