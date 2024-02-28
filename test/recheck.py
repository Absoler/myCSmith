#!/usr/bin/python3

''' recheck the compiler constraints for `output2.c`,
    and report the failed part

'''

default_option=
test_type=
pin_root=
root_dir=

import sys, os
sys.path.append("{}/regression".format(root_dir))
from compilerbugs import pintool, compile

compilers = sys.argv[1].split(":")

success = True
tempdir = "./recheckdir"
os.system("mkdir {} -p".format(tempdir))
for compiler in compilers:
    positive = 0 if compiler.startswith("~") else 1
    compiler = compiler[1:] if not positive else compiler

    if '+' in compiler:
        compiler, extra_opt = compiler.split('+')
        extra_opt += " " + "-w -gdwarf-4"
    else:
        extra_opt = "-w -gdwarf-4"
    ret = compile(compiler, extra_opt, "./output2.c", "output2")
    if ret != 0:
        print("fail to compile with {}".format(compiler))
        exit(1)
    ret = pintool("./output2")
    if ret != positive:
        success = False
        break

    if positive:
        if not os.path.exists("{}/descript.positive".format(tempdir)):
            os.system("cp ./descript.out {}/descript.positive".format(tempdir))
        else:
            res = os.system("{}/test/compare.py descript.out {}/descript.positive".format(root_dir, tempdir))
            if res != 0:
                success = False
                break
    else:
        pass

os.system("rm {} -r".format(tempdir))
if success:
    print("pass compiler constraints check!")
    exit(0)
else:
    print("{} not pass check".format(compiler))
    exit(1)