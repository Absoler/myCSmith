#!/usr/bin/python3

import sys, os, re
from enum import Enum


Stage = Enum("Stage", ('DEFINE', 'FUNCTIONS', 'MAIN'))

if len(sys.argv) == 1:
    inpath, outpath = "./output2.c", "output2.c"
elif len(sys.argv) == 2:
    inpath, outpath = sys.argv[1], sys.argv[1]
else:
    inpath, outpath = sys.argv[1], sys.argv[2]
infile = open(inpath, "r")

re_getvar = re.compile(r'(g_\d+)')
define = []
re_getdecl = re.compile(r'^(const)?\s*(u?int(8|16|32|64)_t|struct\s*S\d+|union\s*U\d+)(\s*(const)?\s*\**\s*)*(g_\d+)')
functions = ""
main = []
re_setinfo = re.compile(r'setInfo\(\(unsigned long\)\(&(g_\d+)')
re_setreadcnt = re.compile(r'setReadCnt\(\(unsigned long\)\(&(g_\d+)')
re_side = re.compile(r'side=\(side\+\(unsigned long\)&?(g_\d+)')

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

deadvars, refvars = [], set()
for line in reversed(define):
    mat = re_getdecl.match(line)
    if not mat:
        continue

    var = mat.group(6)
    if var not in functions and var not in refvars:
        deadvars.append(var)
    else:
        vars = re_getvar.findall(line)
        vars.remove(var)
        refvars.update(vars)


infile.close()
outfile = open(outpath, "w")

for line in define:
    mat = re_getdecl.match(line)
    if mat and mat.group(6) in deadvars:
        continue
    outfile.write(line)

outfile.write(functions)

for line in main:    
    if line.lstrip().startswith("setInfo"):
        var = re_setinfo.match(line.lstrip()).group(1)
        if var in deadvars:
            continue

    elif line.lstrip().startswith("setReadCnt"):
        var = re_setreadcnt.match(line.lstrip()).group(1)
        if var in deadvars:
            continue

    elif line.lstrip().startswith("side=(side+(unsigned long)g_") or\
        line.lstrip().startswith("side=(side+(unsigned long)&"):
        var = re_side.match(line.lstrip()).group(1)
        if var in deadvars:
            continue
    
    outfile.write(line)

outfile.close()