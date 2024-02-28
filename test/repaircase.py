#!/usr/bin/python3

''' 
    1. for O3 testing, "#pragma GCC optimize ("O0")" must be set before `main` to
        avoid `setReadCnt` is optimized unexpectedly
    2. replace "csmith.h" with "my_safe_math.h"
'''

import sys, os
from multiprocessing import Pool

def repaircases_rt(num, casePaths:list):
    cnt = 0
    for casePath in casePaths:
        if cnt % 5000 == 0:
            print("Process {} repair {} cases".format(num, cnt))
        with open(casePath, "r") as file:
            lines = file.readlines()
        
        new_lines = []
        for line in lines:
            if "csmith.h" in line:
                continue

            elif "stdlib" in line:
                new_lines.append("#include \"my_safe_math.h\"\n")
            
            elif "main" in line:
                new_lines.append("#pragma GCC optimize (\"O0\")\n")
            
            new_lines.append(line)

        # remove repeat lines
        lines = new_lines
        new_lines = []
        for i, line in enumerate(lines):
            if i > 0 and line == lines[i-1]:
                continue
            new_lines.append(line)

        with open(casePath, "w") as file:
            file.writelines(new_lines)
        
        cnt += 1

if __name__ == "__main__":
    targetDir = sys.argv[1]
    nproc = int(sys.argv[2])

    allcasePaths = os.listdir(targetDir)
    allcasePaths = [casePath for casePath in allcasePaths if casePath.endswith('.c')]
    allcasePaths = [os.path.join(targetDir, casePath) for casePath in allcasePaths]

    pool = Pool(nproc)
    args_lst = [[] for i in range(nproc)]
    for i in range(len(allcasePaths)):
        args_lst[i%nproc].append(allcasePaths[i])
    
    # repaircases_rt(args[0])
    for i in range(nproc):
        pool.apply_async(repaircases_rt, args=(i, args_lst[i]))
    
    pool.close()
    pool.join()