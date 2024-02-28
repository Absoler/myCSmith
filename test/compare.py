#! /usr/bin/python3
#
#   compare two descript.out file and judge whether 
#   the two source C code have the same problem
#

import sys
import copy


class Var:
    def __init__(self, name = "", len = 0, expect = 0, actual = 0, insts = None) -> None:
        self.name = name
        self.len = len
        self.expect = expect
        self.actual = actual
        self.insts = insts if insts else set()
    def __eq__(self, v) -> bool:
        if self.name.strip() != v.name.strip():
            return False
        elif set(self.insts) != set(v.insts):
            return False
        elif self.expect!=v.expect or self.actual!=v.actual:
            return False
        else:
            return True
    def __hash__(self) -> int:
        return hash(self.name) + hash(self.expect) + hash(self.actual) + hash(self.len)
    
    def keys(self):
        return ("name", "len", "expect", "actual", "insts")
    
    def __getitem__(self, key):
        if key == 'insts':
            return list(self.insts)
        else:
            return getattr(self, key)
    
    def __str__(self) -> str:
        s = "var\n"
        s += "{} {}\n".format(self.name, self.len)
        s += "{} {}\n".format(self.expect, self.actual)
        for inst in self.insts:
            s += "{}\n".format(inst)
        return s
    
    def __repr__(self) -> str:
        return self.__str__()

    @classmethod
    def fromdict(cls, data):
        return cls(**data)
    

class Info:
    def __init__(self) -> None:
        self.mores = set()
        self.unexpected = set()
    
    def __eq__(self, other) -> bool:
        return self.mores == other.mores and self.unexpected == other.unexpected
    
    def __hash__(self) -> int:
        mores_hsh = 0
        for var in self.mores:
            mores_hsh += hash(var)
        unexpected_hsh = 0
        for var in self.unexpected:
            unexpected_hsh += hash(var)
        return mores_hsh * unexpected_hsh

    def keys(self):
        return ("mores", "unexpected")
    
    def __getitem__(self, key):
        if key == "mores" or key == "unexpected":
            return list(map(dict, list(getattr(self, key))))
        else:
            return getattr(self, key)
    
    def __str__(self) -> str:
        s = "more\n"
        for var in self.mores:
            s += var.__str__()
        s += "unexpected"
        for var in self.unexpected:
            s += var.__str__()
        return s
    
    def __repr__(self) -> str:
        return self.__str__()
    
    @classmethod
    def fromdict(cls, data):
        info = cls()

        for var in data["mores"]:
            info.mores.add(Var.fromdict(var))
        for var in data["unexpected"]:
            info.unexpected.add(Var.fromdict(var))
        
        return info



def parse(fileName: str = "./descript.out"):
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

if __name__ == "__main__":
    assert(len(sys.argv)==3)
    curInfo = parse(sys.argv[1])
    refInfo = parse(sys.argv[2])
    if curInfo == refInfo:
        exit(0)
    else:
        exit(1)
# for v in curInfo.mores:
#     print(v.name)
#     print(v.expect, v.actual)
#     print(v.insts)
# for v in refInfo.mores:
#     print(v.name)
#     print(v.insts)
# print(curInfo.mores)
