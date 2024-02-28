#! /usr/bin/python3
import argparse
import re
import os
import sys
import multiprocessing as mp
from compilerbugs import pintool

default_option=
test_type=
pin_root=
root_dir=


def gencases(pnum:int, mod:int, limit:int, genbase:int):
    ids = [i*mod + pnum + genbase for i in range(int(limit/mod))]
    for id in ids:
        os.system("{}/build/src/csmith {} --no-bitfields --no-volatiles --probability-configuration {}/prob.txt  -o caserepo/output{}.c 1>/dev/null".format(root_dir, type_option, root_dir, id))
        if (id-pnum) % 1000 == 0:
            print("generate {} cases".format(id))

def runtests(ids:list, resfile_lock):
    tempdir = os.popen("mktemp -d").read().strip()
    os.chdir(tempdir)
    for id in ids:
        casepath = "{}/output{}.c".format(tempdir, id)
        os.system("cp {}/regression/caserepo/output{}.c {}".format(root_dir, id, casepath))
        if not os.path.exists(casepath):
            print("ERROR: missing " + casepath, file=sys.stderr)
        os.system("{} {} -I{}/runtime {} 2>/dev/null".format(compiler, casepath, root_dir, default_option))
        res = pintool("./a.out")
        if res == 1:
            resfile_lock.acquire()
            with open(respath, "a") as f:
                f.write("{}\n".format(id))
            resfile_lock.release()
            os.system("cp descript.out {}/regression/descripts/{}_{}descript.out".format(root_dir, compiler, id))
        os.system("rm {}".format(casepath))
        os.system("rm {}/core*".format(tempdir))

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="some practicals functions for test")
    # generation part
    parser.add_argument("--generate", "-gen", default=-1, help="only generate [NUM] test cases in ./caserepo", type=int)
    parser.add_argument("--base", "-b", default=0, type=int, help="indicate where generated cases number from")
    # test part
    parser.add_argument("--range", "-r", default="-1", help="specify test range, `integer` or `integer-integer` which indicates [lo, hi)")
    parser.add_argument("--test", default="", help="specify compiler, count number of triggered case, use -gen to specify test range")
    parser.add_argument("--nproc", "-n", default=1, type=int, help="allow n jobs at once, available for generate")

    args = parser.parse_args()

    n = int(args.nproc)
    gen = int(args.generate)
    base = int(args.base)
    # generate test cases
    if gen != -1:
        if test_type == "0":
            type_option = ""
        else:
            type_option = "--store"
        if not os.path.exists("./caserepo"):
            os.mkdir("./caserepo")
        # if `n` doesn't divide `gen`, generate int(gen/n)*n cases
        jobs = []
        for i in range(n):
            jobs.append(mp.Process(target=gencases, args=(i, n, gen, base)))
            jobs[i].start()
        for i in range(n):
            jobs[i].join()
        exit(0)

    # run compiler tests
    if args.test:
        part_flag = "-" in args.range
        lo, hi = 0, -1
        if part_flag:
            lo, hi = map(int, args.range.split("-"))
        else:
            hi = int(args.range)

        assert(hi != -1)
        compiler = args.test
        if not os.path.exists("./caserepo"):
            print("ERROR: no ./caserepo directory", file=sys.stderr)

        if not os.path.exists("./descripts"):
            os.mkdir("descripts")
        
        if part_flag:
            respath = "{}/regression/{}_{}-{}.res".format(root_dir, compiler, lo, hi)
        else:
            respath = "{}/regression/{}.res".format(root_dir, compiler)
        if os.path.exists(respath):
            print("ERROR: res file exists! can't overwrite", file=sys.stderr)
            exit(1)
        with open(respath, "w") as f:
            f.write("{}\n".format(compiler))

        oldpath = os.getcwd()
        
        
        ids_arr = [ [] for i in range(n)]
        for i in range(lo, hi):
            ids_arr[i%n].append(i)

        resfile_lock = mp.Manager().Lock()
        jobs = []
        for i in range(n):
            jobs.append(mp.Process(target=runtests, args=(ids_arr[i], resfile_lock)))
            jobs[i].start()
        for i in range(n):
            jobs[i].join()
        with open(respath, "r") as f:
            triggercnt = len(f.readlines()) - 1
        print("trigger {}/{}".format(triggercnt, hi - lo))

        # os.system("rm {} -r".format(tempdir))
        exit(0)
    
