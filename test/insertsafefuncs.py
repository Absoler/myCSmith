#!/usr/bin/python3
import sys, re, json
from enum import Enum

re_safefunc = re.compile(r'(safe_[^\(]+)')
Stage = Enum("Stage", ('DEFINE', 'FUNCTIONS', 'MAIN'))


with open("safefuncs.json", "r") as f:
    safe_func_dict = json.load(f)

if len(sys.argv) == 1:
    inpath, outpath = "./output2.c", "output2.c"
elif len(sys.argv) == 2:
    inpath, outpath = sys.argv[1], sys.argv[1]
else:
    inpath, outpath = sys.argv[1], sys.argv[2]
infile = open(inpath, "r")

define = []
functions = ""
main = []

stage = Stage.DEFINE
for line in infile.readlines():
    if stage == Stage.DEFINE:
        define.append(line)
    elif stage == Stage.FUNCTIONS:
        functions += line
    else:
        main.append(line)

    if "FORWARD DECLARATIONS" in line:
        stage = Stage.FUNCTIONS
    
    elif line == "unsigned long side=0;\n":
        stage = Stage.MAIN

infile.close()
outfile = open(outpath, "w")

need_safefuncs = set(re_safefunc.findall(functions))
for line in define:
    if "my_safe_math.h" in line or "csmith.h" in line:
        continue
    outfile.write(line)

for safefunc in need_safefuncs:
    outfile.write(safe_func_dict[safefunc])

outfile.write(functions)
for line in main:
    outfile.write(line)

outfile.close()