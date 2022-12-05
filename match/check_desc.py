#! /usr/bin/python3
#
#   compare two descript.out file and judge whether 
#   the two source C code have the same problem
#
import sys
import copy

assert(len(sys.argv)==3)

class Var:
    def __init__(self) -> None:
        self.name = ""
        self.len = 0
        self.expect = 0
        self.actual = 0
        self.insts = set()
    def __eq__(self, v) -> bool:
        if self.name.strip() != v.name.strip():
            return False
        elif self.insts != v.insts:
            return False
        elif self.expect!=v.expect or self.actual!=v.actual:
            return False
        else:
            return True
    def __hash__(self) -> int:
        return hash(self.name)
    

class Info:
    def __init__(self) -> None:
        self.mores = set()
        self.unexpected = set()
    
    def suspect_moreOnce_in_for(self) -> bool:
        for var in self.mores:
            if not (int(var.expect) > 1 and int(var.actual) - int(var.expect) == 1):
                return False
        return True

def parse(fileName: str):
    file = open(fileName, "r")
    flag = 0

    info = Info()
    while 1:
        line = file.readline()
        if line.strip() == "unexpected":
            flag = 1
        elif line.strip() == "more":
            continue
        elif line.strip() == "var":
            var = Var()
            line = file.readline()
            name, len = line.split()
            var.name = name
            var.len = len

            line = file.readline()
            expect, actual = line.split()
            var.expect = expect
            var.actual = actual

            while 1:
                line = file.readline()
                if line.strip() == "done":
                    if flag == 0:
                        info.mores.add(copy.deepcopy(var))
                    else:
                        info.unexpected.add(copy.deepcopy(var))
                    del var
                    break
                else:
                    var.insts.add(line.strip())
        elif line == "":
            break
        else:
            continue
    file.close()
    return info




curInfo = parse(sys.argv[1])

f = getattr(curInfo, sys.argv[2])
bingo = f()
if bingo:
    exit(1)


# for v in curInfo.mores:
#     print(v.name)
#     print(v.expect, v.actual)
#     print(v.insts)
# for v in refInfo.mores:
#     print(v.name)
#     print(v.insts)
# print(curInfo.mores)
