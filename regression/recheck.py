#!/usr/local/bin/python3

import sys, os

opt_option=
test_type=
pin_root=
root_dir=

respath = sys.argv[1]


with open(respath, "r") as resfile:
    compiler = resfile.readline().strip()
    error = 0
    cnt = 0

    for line in resfile.readlines():
        cnt += 1
        i = int(line.strip())
        casepath = "caserepo/output{}.c".format(i)
        os.system("cp {} output.c".format(casepath))
        if not os.path.exists(casepath):
            print("ERROR: missing " + casepath, file=sys.stderr)
        
        os.system("{} {} {} 2>/dev/null".format(compiler, casepath, opt_option))
        ret = os.system("timeout -s SIGTERM 5s {}/pin -t {}/checkAccess/obj-intel64/checkAccess.so -- ./a.out {} func ./ 1>/dev/null".format(pin_root, root_dir, test_type))
        res = os.popen("cat result.out").read().strip()
        if ret !=0 or res != "1":
            error += 1
            print("WARN: output{}.c not trigger".format(i))
    print("PASS {}/{}".format(cnt - error, cnt))