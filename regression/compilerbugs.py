#!/usr/bin/python3

opt_option=
test_type=
pin_root=
root_dir=

import sys, os
sys.path.append("{}/test".format(root_dir))
import compare
from enum import Enum

CompilerType = Enum("CompilerType", ("gcc", "clang", "both", "invalid"))

def pintool(elf_path):
    ''' -1: runtime error
        0:  no bug
        1:  has bug
    '''
    ret = os.system("timeout -s SIGTERM 5s {}/pin -t {}/checkAccess/obj-intel64/checkAccess.so -- {} {} func ./ 1>/dev/null".format(pin_root, root_dir, elf_path, test_type))
    if ret != 0:
        return -1
    ret = int(os.popen("cat ./result.out").read().strip())
    return ret

class Bug:
    def __init__(self, id:int, info = None) -> None:
        self.id = id
        self.info = info

        # filled by Compiler.init_bugs()
        self.compilerType = CompilerType.invalid
        self.compilerHash = 0   # used for clustering annlysis, explore bugs group
    
    def __eq__(self, bug) -> bool:
        return self.id == bug.id and self.info == bug.info
    
    def __hash__(self) -> int:
        return hash(self.id) + hash(self.info)
    
    def keys(self):
        return ("id", "info", )
    
    def __getitem__(self, key):
        if key == "info":
            return dict(self.info)
        else:
            return getattr(self, key)

    @classmethod
    def fromdict(cls, data):
        return Bug(data["id"], compare.Info.fromdict(data["info"]))
    

class Compiler:
    compiler_map = {}
    gccs = []
    clangs = []
    
    @classmethod
    def add(cls, compiler_name:str):
        if compiler_name not in Compiler.compiler_map:
            Compiler.compiler_map[compiler_name] = Compiler(*compiler_name.split("-"))
        
    @classmethod
    def get(cls, compiler_name:str):
        Compiler.add(compiler_name)
        return Compiler.compiler_map[compiler_name]
    
    @classmethod
    def init_bugs(cls, bugs_map):
        for bug in bugs_map:
            for compiler_name in bugs_map[bug]:
                compiler = Compiler.get(compiler_name)
                compiler.bugs.append(bug)
        compilers = list(cls.compiler_map.values())
        cls.gccs = [compiler for compiler in compilers if compiler.compilerType == CompilerType.gcc]
        cls.gccs.sort()
        cls.clangs = [compiler for compiler in compilers if compiler.compilerType == CompilerType.clang]
        cls.clangs.sort()
        for bug in bugs_map:
            # set which is bug's triggered compiler
            cpStr = ''.join(bugs_map[bug])
            if "gcc" in cpStr and "clang" in cpStr:
                bug.compilerType = CompilerType.both
                bug.compilerHash = 0
                continue
            elif "gcc" in cpStr:
                bug.compilerType = CompilerType.gcc
            elif "clang" in cpStr:
                bug.compilerType = CompilerType.clang
            else:
                assert(0)
            
            # this hash can't distinguish different compilers
            compilerHash = 0
            for compiler_name in bugs_map[bug]:
                compiler = Compiler.get(compiler_name)
                if compiler.compilerType == CompilerType.gcc:
                    id = cls.gccs.index(compiler)
                elif compiler.compilerType == CompilerType:
                    id = cls.clangs.index(compiler)
                compilerHash += (1<<id)

            bug.compilerHash = compilerHash

    @classmethod
    def uncompress(cls, cpHash:int, compilerType):
        res = []
        cps = cls.gccs if compilerType == CompilerType.gcc else cls.clangs
        for i in range(len(cps)):
            if cpHash & (1<<i) != 0:
                res.append(cps[i].name)
        return res
    
    @classmethod
    def isregression(cls, cpHash) -> bool:
        i = 1
        state = 0
        while i < cpHash:
            if i & cpHash != 0:
                if state & 1 == 0:
                    state += 1
            else:
                if state & 1 == 1:
                    state += 1
            i *= 2
        return state > 2
    
    @classmethod
    def hasnewest(cls, cpHash:int, compilerType) -> bool:
        if compilerType == CompilerType.gcc:
            return (1 << (len(Compiler.gccs) - 1)) & cpHash != 0
        elif compilerType == CompilerType.clang:
            return (1 << (len(Compiler.clangs) - 1)) & cpHash != 0
        return False
    
    def __init__(self, prefix:str, version:str) -> None:
        self.compilerType = CompilerType[prefix]
        self.version = version

        ''' assume the version code consists of at most 3 2-digit number
        '''
        self.varr = list(map(int, version.split(".")))
        if len(self.varr) < 3:
            self.varr.extend([0] * (3 - len(self.varr)))
        self.vnum = self.varr[0] * 10**4 + self.varr[1]  * 10**2 + self.varr[2]

        self.bugs = []
    

    def __lt__(self, other):
        return self.vnum < other.vnum

    def __eq__(self, other):
        return self.compilerType == other.compilerType and self.vnum == other.vnum
    
    def ismain(self) -> bool:
        if self.compilerType == CompilerType.clang:
            return self.vnum[1] == 0 and self.vnum[2] == 0
        elif self.compilerType == CompilerType.gcc:
            return self.vnum[1] == 2 and self.vnum[2] == 0
        else:
            return False
    
    @property
    def name(self):
        return self.compilerType.name + "-" + self.version