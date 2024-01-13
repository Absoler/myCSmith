#!/usr/bin/python3

''' recheck the compiler constraints for `output2.c`

'''

opt_option=
test_type=
pin_root=
root_dir=

import sys, os

compilers = sys.argv[1].split(":")

success = True
tempdir = "./recheckdir"
os.system("mkdir {} -p".format(tempdir))
for compiler in compilers:
    negative = 1 if compiler.startswith("~") else 0
    compiler = compiler[1:] if negative else compiler

    ret = os.system("{} output2.c {} -gdwarf-4 -w -o output2".format(compiler, opt_option))
    if ret != 0:
        print("fail to compile with {}".format(compiler))
        exit(1)
    ret = os.system("{}/pin -t {}/checkAccess/obj-intel64/checkAccess.so -- ./output2 {} func ./ >/dev/null".format(pin_root, root_dir, test_type))
    if ret != 0:
        print("fail to run compiled by {}".format(compiler))
    if not negative and not os.path.exists("{}/descript.positive".format(tempdir)):
        os.system("cp ./descript.out {}/descript.positive".format(tempdir))
    else:
        res = os.system("{}/test/compare.py descript.out {}/descript.positive".format(root_dir, tempdir))
        res >>= 8
        if res != negative:
            success = False
            print("descript can't match, {} should {} trigger".format(compiler, "not" if negative else ""))
            break
if success:
    print("pass compiler constraints check!")
    exit(0)
else:
    exit(1)