#! /usr/bin/python3
import argparse
import re
import os
import sys

opt_option=
test_type=
pin_root=
root_dir=

parser = argparse.ArgumentParser(description="some practicals functions for test")

parser.add_argument("--file_range", default="" ,type=str, help="fileNo range")
parser.add_argument("-d", default="./", type=str, help="target directory")
parser.add_argument("--generate", "-gen", default=-1, help="only generate [NUM] test cases in ./caserepo", type=int)
parser.add_argument("--test", default="", help="specify compiler, count number of triggered case, use -gen to specify test range")

def get_file_range(range:str, dir:str):
    res = []
    grp = re.match(r"(\d*)-(\d*)", range)
    start, end = map(int, [
        grp[1] if grp[1] != "" else "0",
        grp[2] if grp[2] != "" else "9999"
        ])
    files = os.listdir(dir)
    for file in files:
        if start <= int(re.match(r"(\d+).*", file)[1]) < end:
            res.append(dir + "/" + file)
    return res


if __name__ == "__main__":
    args = parser.parse_args()
    directory = args.d
    if args.file_range != "":
        files = get_file_range(args.file_range, directory)
        for file in files:
            print(file)

    gen = int(args.generate)
    if gen != -1 and args.test == "":
        if not os.path.exists("./caserepo"):
            os.mkdir("./caserepo")
        for i in range(gen):
            os.system("build/src/csmith --no-safe-math --no-bitfields --no-volatiles --probability-configuration ./prob.txt  -o caserepo/output{}.c 1>/dev/null".format(i))
            if i % 1000 == 0:
                print("generate {} cases".format(i))
    
    if args.test:
        assert(gen != -1)
        compiler = args.test
        if not os.path.exists("./caserepo"):
            print("ERROR: no ./caserepo directory", file=sys.stderr)
        
        if not os.path.exists("./descripts"):
            os.mkdir("descripts")
        triggercnt = 0
        respath = "{}/regression/{}.res".format(root_dir, compiler)
        if os.path.exists(respath):
            print("ERROR: res file exists! can't overwrite", file=sys.stderr)
            exit(1)
        with open(respath, "w") as f:
            f.write("{}\n".format(compiler))

        oldpath = os.getcwd()
        tempdir = os.popen("mktemp -d").read().strip()
        os.chdir(tempdir)
        for i in range(gen):
            casepath = "{}/output{}.c".format(tempdir, i)
            os.system("cp {}/regression/caserepo/output{}.c {}".format(root_dir, i, casepath))
            if not os.path.exists(casepath):
                print("ERROR: missing " + casepath, file=sys.stderr)
            os.system("{} {} {} 2>/dev/null".format(compiler, casepath, opt_option))
            ret = os.system("timeout -s SIGTERM 5s {}/pin -t {}/checkAccess/obj-intel64/checkAccess.so -- ./a.out {} func ./ 1>/dev/null".format(pin_root, root_dir, test_type))
            res = os.popen("cat result.out").read().strip()
            if ret == 0 and res == "1":
                triggercnt += 1
                with open(respath, "a") as f:
                    f.write("{}\n".format(i))
                os.system("cp descript.out {}/regression/descripts/{}_{}descript.out".format(root_dir, compiler, i))
        print("{} / {} triggered".format(triggercnt, gen))
        os.chdir(oldpath)
        os.system("rm {} -r".format(tempdir))
        print(tempdir)