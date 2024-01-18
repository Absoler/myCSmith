#!/usr/bin/python3

''' re-confirm the bugs recorded in `.res` file generate by `tool.py`'s test procedure

'''

import sys, os
import argparse
from compilerbugs import pintool

opt_option=
test_type=
pin_root=
root_dir=

parser = argparse.ArgumentParser(description="check whether reported cases can trigger")
parser.add_argument("--caseid", "-id", help="the case id to be checked", default="")
parser.add_argument("--compiler", "-c", help="the compiler used, can be docker image, distinguished by the `:` in it", default="")

parser.add_argument("--respath", "-r", help="the res file to be checked", default="")
args = parser.parse_args()

assert(args.caseid and args.compiler or args.respath)

if args.respath:
    resfile = open(args.respath, "r")
    compiler = resfile.readline().strip()
    caseids = resfile.readlines()
    caseids = [int(id.strip()) for id in caseids]
else:
    compiler = args.compiler
    caseids = [int(args.caseid)]

error = 0
cnt = 0

oldpath = os.getcwd()
tempdir = "{}/regression/checktempdir".format(root_dir)
os.system("mkdir {} -p".format(tempdir))
os.chdir(tempdir)
for i in caseids:
    cnt += 1
    casepath = "{}/output.c".format(tempdir, i)
    os.system("cp {}/regression/caserepo/output{}.c {}".format(root_dir, i, casepath))
    if not os.path.exists(casepath):
        print("ERROR: missing " + casepath, file=sys.stderr)
    
    if not ":" in compiler:
        os.system("{} {} -I{}/runtime/ {} 2>/dev/null".format(compiler, casepath, opt_option))
    else:
        source_dir = ""
        if os.path.exists("{}/regression/docker.volume".format(root_dir)):
            mountinfo = open("{}/regression/docker.volume".format(root_dir), "r").readline().strip()
            hostdir, containerdir = mountinfo.split(":")
            if os.path.samefile(os.path.commonprefix([containerdir, root_dir]), containerdir):
                source_dir = os.path.join(hostdir, os.path.relpath(tempdir, containerdir))
        if source_dir == "":
            print("ERROR: no valid test path, should be accessible from the host")
            exit(1)
        compiler_prefix = compiler.split(":")[0]
        os.system("docker run --rm -v {}:/root -w /root {} {} /root/output.c {} -g".format(source_dir, compiler, compiler_prefix, opt_option))
    
    res = pintool("./a.out")
    if res != 1:
        error += 1
        print("WARN: output{}.c not trigger".format(i))
print("PASS {}/{}".format(cnt - error, cnt))
if len(caseids) > 1:
    os.system("rm {} -r".format(tempdir))
print(tempdir)