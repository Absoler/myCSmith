#include "pin.H"
#include<string>
#include<cstdio>
#include<algorithm>
#include<set>
#include<map>
#include<cstring>
#include<vector>
#include<utility>
#include<utility>
#include<assert.h>
using std::string;
using std::map;
using std::upper_bound;
using std::sort;
using std::vector;
using std::set;
using std::pair;
#define VERSION_2
// define COPY_EXPECTED_ACCESS_INFO or PASS_EXPECTED_ACCESS_INFO to decide how to get message about expected accesses. version1 + copy... is wrong
#define PASS_EXPECTED_ACCESS_INFO
#define MAX_GLOBAL_VAR_NUM 1000
#define MAX_ACCESS_NUM 2000

string targetFuncPrefix;
string result_file_prefix;
bool isWrite;
FILE* outfile;
FILE* resfile;
FILE* descriptFile;
bool fail=false;

struct Var{	//record a var
	ADDRINT addr;
    UINT32 size;
    char name[10];
}globals[MAX_GLOBAL_VAR_NUM];
bool comp_Var(const Var& var1, const Var& var2){
	    return var1.addr<var2.addr;
}
int cnt_globals=0;

//-------util----------------
void print_accesses(FILE* file);
void print_globals(FILE* file);

#ifdef PASS_EXPECTED_ACCESS_INFO
//---------get global info------------
VOID record_var(ADDRINT _addr, ADDRINT cnt){
    // printf("pin info-addr: %lu    cnt: %ld\n",_addr,cnt);
    if(cnt>MAX_GLOBAL_VAR_NUM){
        printf("too many global variables!\n");
        exit(1);
    }
    PIN_SafeCopy(globals,Addrint2VoidStar(_addr),sizeof(Var)*cnt);
    cnt_globals=cnt;
    sort(globals, globals+cnt, comp_Var);
}

VOID hack_getInfo(RTN rtn, VOID* v){
	RTN_Open(rtn);
	if(RTN_Name(rtn)=="getInfo"){
		RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)record_var,
			IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
			IARG_FUNCARG_ENTRYPOINT_VALUE, 1, IARG_END);
	}
	RTN_Close(rtn);
}
#endif

#ifdef VERSION_1
struct Access{	//record a access operation
	ADDRINT start, ins_ptr; 
    UINT32 length;
	BOOL isGlobal;
    char name[10];
};
int access_limit=10000;
vector<Access> accesses(access_limit);

bool comp_Access(const Access& access1, const Access& access2){
    if(access1.isGlobal&&(!access2.isGlobal)){
        return true;
    }else if((!access1.isGlobal)&&access2.isGlobal){
        return false;
    }else{
    	return access1.start<access2.start;
    }
}
int cnt_accesses=0;

void print_accesses(FILE* file){
    fprintf(file, "all access memory\n");
    for(int i=0;i<cnt_accesses;i++){
        fprintf(file, "0x%lx %d %s\n", accesses[i].start, accesses[i].length, (accesses[i].isGlobal?"global":"local"));
    }
    fprintf(file,"\n");
}

map<ADDRINT, string> disasMap;

//----------record access info-----------
VOID record_Access(ADDRINT ip, ADDRINT _start, UINT32 _length) { 
    if(cnt_accesses>=access_limit-1){
        if(access_limit>=5e5){
            printf("too many accesses!(>500000)");
            exit(1);
        }
        printf("%d accesses!\n",access_limit);
        access_limit+=10000;
        accesses.resize(access_limit);
    }
    accesses.push_back(Access());
    accesses[cnt_accesses].start=_start;
    accesses[cnt_accesses].length=_length;
    accesses[cnt_accesses].ins_ptr= ip;
    accesses[cnt_accesses].isGlobal=false;
    cnt_accesses++;
}

VOID hack_targetFunc(RTN rtn, VOID* v){
    RTN_Open(rtn);
    // printf("%s\n",RTN_Name(rtn).substr(0,targetFuncPrefix.length()).c_str());
    if(RTN_Name(rtn).substr(0,targetFuncPrefix.length())==targetFuncPrefix){
        
		printf("%s\n",RTN_Name(rtn).c_str());
        for (INS ins = RTN_InsHead(rtn); INS_Valid(ins); ins = INS_Next(ins)){
            if (INS_IsMemoryAccess(ins)){
                disasMap[INS_Address(ins)] = INS_Disassemble(ins);
                INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)record_Access, IARG_INST_PTR, IARG_MEMORYREAD_EA, IARG_MEMORYREAD_SIZE, IARG_END);
            }
        }    
    }
    RTN_Close(rtn);
}

//----------analysis--------
VOID Fini(INT32 code, VOID* val){
	sort(globals, globals+cnt_globals, comp_Var);
    //identify all global accesses
    for(int i=0;i<cnt_accesses;i++){
        // globals[id] 's address is the maximum less than or equal to accesses[i].start
        int id=upper_bound(globals, globals+cnt_globals, (Var){accesses[i].start, 0}, comp_Var)-globals-1;
        if(id>=0&&id<cnt_globals){
            // mark global access
            accesses[i].isGlobal = (accesses[i].start<globals[id].addr+globals[id].size && accesses[i].start+accesses[i].length<=globals[id].addr+globals[id].size);
            if(accesses[i].isGlobal){
                strcpy(accesses[i].name,globals[id].name);
            }
        }
    }
    print_globals(outfile);
    fprintf(outfile,"\n\noverlap:\n");
    // check whether accesses overlap
    /* Supports detecting multiple executions of the same statement, */
    // sort(accesses, accesses+cnt_accesses, comp_Access);
    // bool overlap=false;
    // for(int i=0; i<cnt_accesses-1; i++){
    //     if(accesses[i+1].isGlobal){
    //         overlap=(accesses[i].start+accesses[i].length>accesses[i+1].start&&accesses[i+1].isGlobal);  //next access occur in current access range
    //         if(overlap){
    //             fprintf(outfile, "0x%lx: %s 0x%lx: %s len:%u    |    0x%lx: %s 0x%lx: %s len:%u\n", accesses[i].ins_ptr, disasMap[accesses[i].ins_ptr].c_str(), accesses[i].start, accesses[i].name, accesses[i].length,
    //                                                                                                 accesses[i+1].ins_ptr, disasMap[accesses[i+1].ins_ptr].c_str(), accesses[i+1].start, accesses[i+1].name, accesses[i+1].length);
    //         }                                                                            
    //     }
    // }
    bool overlap=false;
    for(int i=0;i<cnt_accesses-1;i++){
        if(!accesses[i].isGlobal){
            continue;
        }
        for(int j=i+1;j<cnt_accesses;j++){
            if(!accesses[j].isGlobal){
                continue;
            }
            if(accesses[i].ins_ptr==accesses[j].ins_ptr){
                //only consider overlap from different instructions
                continue;
            }
            if(accesses[i].start<accesses[j].start){
                //overlap occurs when front access spans back access
                overlap=(accesses[i].start+accesses[i].length>accesses[j].start);
            }else if(accesses[i].start>accesses[j].start){
                overlap=(accesses[j].start+accesses[j].length>accesses[i].start);
            }else{
                overlap=true;
            }
            if(overlap){
                fail=true;
                fprintf(outfile, "0x%lx: %s 0x%lx: %s len:%u    |    0x%lx: %s 0x%lx: %s len:%u\n", accesses[i].ins_ptr, disasMap[accesses[i].ins_ptr].c_str(), accesses[i].start, accesses[i].name, accesses[i].length,\
                                                                                                    accesses[j].ins_ptr, disasMap[accesses[j].ins_ptr].c_str(), accesses[j].start, accesses[j].name, accesses[j].length);
            }
        }
    }
    fprintf(resfile,(fail?"1":"0"));
    print_accesses(stdout);
}

#endif

#ifdef VERSION_2
typedef pair<ADDRINT, uint32_t> Access;

// a access is represented as <addr, len>
// PROBLEM: under this strict definition, if compiler split a access in half or combine two accesses, then we cant't identifiy it 
map<Access, int> expect_counter;    // contain all global vars access, and their expected access times (up-limit for now)
map<Access, int> actual_counter;    // record actual access times of each accesses 
set<Access> unexpectedAccesses;      
set<Access> actual_clusters[MAX_GLOBAL_VAR_NUM];    // contain all actual acesses belonging to corresponding var, (may not overlapped with each other, only belong to the same var)
set<Access> expect_clusters[MAX_GLOBAL_VAR_NUM];    // contain all expected acesses of corresponding var
map<Access, set<ADDRINT>> instOfAccess;    // record all insts addrs a access occur
map<Access, int> varOfAccess;              // record which var the access belongs to (record index)
map<Access, bool> isForVars;    // record whether a access is forVar, maybe not necessary now, 'cause we combine two situation
map<Access, bool> isGlobals;    // record whether a access located in global, not necessary too.

map<ADDRINT, string> disasMap;  // store instruction text
map<ADDRINT, string> funcMap;   // store funcName inst belongs
map<ADDRINT, pair<int, int>> locMap;    // store src location the inst belongs
map<ADDRINT, string> typeMap;   // store inst's mnemonic

// get the global var that cover `access`
inline int getInd(const Access& access){
    Var tmp = {access.first, access.second, ""};
    return upper_bound(globals, globals+cnt_globals, tmp, comp_Var)-globals-1;
}

inline bool partOf(const Access& access, const Var& var){
    return access.first>=var.addr && access.first+access.second<=var.addr+var.size;
}
inline bool partOf(const Access& access1, const Access& access2){
    return (access1.first>=access2.first && access1.first+access1.second<=access2.first+access2.second);
}

bool isGlobal(const Access &access){
    // ADDRINT start=access.first;
    // unsigned len=access.second;
    
    int id=getInd(access);
    bool res=false;
    if(id>=0 && id<cnt_globals){
        res = partOf(access, globals[id]);
    }
    return res;
}

set<Access> findContainers(const Access& target, const set<Access>& accesseset){
    set<Access> res;
    for(auto access:accesseset){
        if(partOf(target, access)){
            res.insert(access);
        }
    }
    return res;
}

#ifdef PASS_EXPECTED_ACCESS_INFO
//----------get vars' expect--------------
VOID record_AccessCnt(ADDRINT addr, int len, int cnt){
    printf("expect access %lx %d %d\n", addr, len, cnt);
    Access access=std::make_pair<ADDRINT, int>(addr, len);
    expect_counter[access]+=cnt;
}
VOID hack_setAccessCnt(RTN rtn, VOID* v){
    RTN_Open(rtn);
    string funcName = RTN_Name(rtn);
    if(funcName.find("setReadCnt") != funcName.npos){
        printf("%s\n", funcName.c_str());
        RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)record_AccessCnt,
            IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
            IARG_FUNCARG_ENTRYPOINT_VALUE, 1,
            IARG_FUNCARG_ENTRYPOINT_VALUE, 2, IARG_END);
    }
    RTN_Close(rtn);
}
#endif

#ifdef COPY_EXPECTED_ACCESS_INFO

struct Expect{
    ADDRINT addr;
    UINT32 length;
    int cnt;
}expects[MAX_ACCESS_NUM];
int cnt_expects = 0;


////----------get vars' decls and expected accesses--------------
VOID record_info(ADDRINT varInfosAddr, ADDRINT varCnt, ADDRINT accessesAddr, ADDRINT accessCnt){
    if(varCnt>MAX_GLOBAL_VAR_NUM){
        printf("too many global variables!\n");
        exit(1);
    }
    PIN_SafeCopy(globals,Addrint2VoidStar(varInfosAddr),sizeof(Var)*varCnt);
    cnt_globals = varCnt;
    sort(globals, globals+varCnt, comp_Var);

    if(accessCnt>MAX_ACCESS_NUM){
        printf("too many accesses!\n");
        exit(1);
    }
    PIN_SafeCopy(expects, Addrint2VoidStar(accessesAddr),sizeof(Expect)*accessCnt);
    cnt_expects = accessCnt;
    for (unsigned i=0; i<accessCnt; i++){
        Access access = std::make_pair<ADDRINT, int>(expects[i].addr, expects[i].length);
        expect_counter[access] += expects[i].cnt;
    }
    printf("expected access count: %lu\n", accessCnt);
}

VOID hack_getInfo(RTN rtn, VOID* v){
	RTN_Open(rtn);
	if(RTN_Name(rtn)=="getInfo"){
		RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)record_info,
			IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
			IARG_FUNCARG_ENTRYPOINT_VALUE, 1, 
            IARG_FUNCARG_ENTRYPOINT_VALUE, 2,
			IARG_FUNCARG_ENTRYPOINT_VALUE, 3, IARG_END);
	}
	RTN_Close(rtn);
}

#endif

//----------record access info-----------
VOID record_Access(ADDRINT ip, ADDRINT start, UINT32 len) { 
    Access access=std::make_pair(start, len);
    // printf("%lx\n", start);
    // printf("actual access %lx %u\n", start, len);
    if(isGlobal(access)){
        printf("actual access %lx %u\n", start, len);
        actual_counter[access]++;
        if(instOfAccess[access].find(ip)==instOfAccess[access].end()){
            // we don't know this inst load this content before
            instOfAccess[access].insert(ip);
        }
    }
}

VOID hack_targetFunc(RTN rtn, VOID* v){
    RTN_Open(rtn);
    string funcName = RTN_Name(rtn);
    if (funcName.find(targetFuncPrefix) !=  funcName.npos) {
        printf(" %s\n", funcName.c_str());
        for (INS ins = RTN_InsHead(rtn); INS_Valid(ins); ins = INS_Next(ins)){
            if ((isWrite && INS_IsMemoryWrite(ins)) || ((!isWrite) && INS_IsMemoryRead(ins))){
                disasMap[INS_Address(ins)] = INS_Disassemble(ins);
                funcMap[INS_Address(ins)] = RTN_Name(rtn);
                typeMap[INS_Address(ins)] = INS_Mnemonic(ins);
                int row,col;
                PIN_GetSourceLocation(INS_Address(ins), &col, &row, NULL);
                // printf("access at %d:%d   0x%lx:%s\n", row, col, INS_Address(ins), INS_Disassemble(ins).c_str());
                locMap[INS_Address(ins)]=pair<int,int>(row, col);
                if(isWrite){
                    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)record_Access, IARG_INST_PTR, IARG_MEMORYWRITE_EA, IARG_MEMORYWRITE_SIZE, IARG_END);
                }else{
                    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)record_Access, IARG_INST_PTR, IARG_MEMORYREAD_EA, IARG_MEMORYREAD_SIZE, IARG_END);
                }
            }
        }    
    }
    RTN_Close(rtn);
}

//----------analysis--------
VOID Fini(INT32 code, VOID* val){
    printf("nothing at start of fini\n");
    fprintf(outfile, (isWrite?"test introduced stores\n\n":"test introduced loads\n\n"));
    print_globals(outfile);
    // build relations between global vars and global accesses
    
    for(auto p:actual_counter){
        Access access=p.first;
        int id=getInd(access);
        assert(partOf(access, globals[id]));
        varOfAccess[access]=id;
        actual_clusters[id].insert(access);
    }
    for(auto p:expect_counter){
        Access access=p.first;
        int id=getInd(access);
        assert(partOf(access, globals[id]));
        varOfAccess[access]=id;
        expect_clusters[id].insert(access);
    }

    int row, col;
    fprintf(outfile,"\n\nproblem of too many accesses:\n");
    fprintf(descriptFile, "more\n");
    for(int id=0;id<cnt_globals;id++){
        if(actual_clusters[id].size()>0){
            printf("%s has %lu accesses\n", globals[id].name, actual_clusters[id].size());
            for(Access access:actual_clusters[id]){
                set<Access> actual_set=findContainers(access, actual_clusters[id]);
                set<Access> expect_set=findContainers(access, expect_clusters[id]);
                if(expect_set.empty()){
                    unexpectedAccesses.insert(access);
                    continue;
                }
                int actual_cnt=0, expect_cnt=0;
                for(Access access:actual_set){
                    actual_cnt+=actual_counter[access]; 
                }
                for(Access access:expect_set){
                    expect_cnt+=expect_counter[access];
                }
                if(actual_cnt>expect_cnt){
                    
                    fprintf(outfile, "%s: start at 0x%lx of len = %d\n", globals[varOfAccess[access]].name, access.first, access.second);
                    fprintf(outfile, "expected %d but access %d\n", expect_cnt, actual_cnt);
                    fprintf(outfile, "insts of main access:\n");
                    fprintf(descriptFile, "var\n");
                    fprintf(descriptFile, "%s %d\n", globals[varOfAccess[access]].name, access.second);
                    fprintf(descriptFile, "%d %d\n", expect_cnt, actual_cnt);
                    for(ADDRINT ip: instOfAccess[access]){
                        row=locMap[ip].first;
                        col=locMap[ip].second;
                        fprintf(outfile, "  %s:%d:%d  %lx  %s\n", funcMap[ip].c_str(), row, col, ip, disasMap[ip].c_str());
                        fprintf(descriptFile, "%s\n", typeMap[ip].c_str());
                    }
                    fprintf(outfile,"insts of other accesses\n");
                    for(Access otherAccess:actual_set){
                        if(otherAccess==access){
                            continue;
                        }
                        fprintf(outfile, "  start at 0x%lx of len = %d\n", otherAccess.first, otherAccess.second);
                        for(ADDRINT ip: instOfAccess[otherAccess]){
                            row=locMap[ip].first;
                            col=locMap[ip].second;
                            fprintf(outfile, "  %s:%d:%d  %lx  %s\n", funcMap[ip].c_str(), row, col, ip, disasMap[ip].c_str());
                            fprintf(descriptFile, "%s\n", typeMap[ip].c_str());
                        }
                        fprintf(outfile, "\n");
                    }
                    fprintf(descriptFile, "done\n");
                    fprintf(outfile, "\n");
                    fail=true;
                }else{
                    printf("    %d/%d\n", actual_cnt, expect_cnt);
                }
            }
        }
    }
    
    
    fprintf(outfile, "problem of unexpected accesses\n");
    fprintf(descriptFile, "unexpected\n");
    for(Access access: unexpectedAccesses){
        fprintf(descriptFile, "var\n");
        fprintf(outfile, "%s: start at 0x%lx of len = %d\n", globals[varOfAccess[access]].name, access.first, access.second);
        fprintf(descriptFile, "%s %d\n", globals[varOfAccess[access]].name, access.second);
        fprintf(descriptFile, "0 -1\n");    // for unexpected cases, assign actual to -1
        for(ADDRINT addr:instOfAccess[access]){
            row=locMap[addr].first;
            col=locMap[addr].second;
            fprintf(outfile, "  %s:%d:%d  %lx  %s\n", funcMap[addr].c_str(), row, col, addr, disasMap[addr].c_str());
            fprintf(descriptFile, "%s\n", typeMap[addr].c_str());
        }
        fprintf(outfile, "\n");
        fprintf(descriptFile, "done\n");
        // fail=true;
    }
    
    fprintf(resfile,(fail?"1":"0"));
    printf("nothing at end of fini\n");

}

#endif

INT32 Usage(){

    return -1;
}


int main(int argc, char* argv[]){
    isWrite = (argv[argc-3][0]=='0'?false:true);
    targetFuncPrefix=argv[argc-2];
    result_file_prefix = argv[argc-1];
    printf("target: %s %d\n", targetFuncPrefix.c_str(), argc);
    PIN_InitSymbols();
    outfile=fopen((result_file_prefix + "checkAccess.out").c_str(), "w");
    resfile=fopen((result_file_prefix + "result.out").c_str(), "w");
    descriptFile=fopen((result_file_prefix + "descript.out").c_str(), "w");
    if(PIN_Init(argc, argv)) return Usage();

    RTN_AddInstrumentFunction(hack_getInfo, 0);
    RTN_AddInstrumentFunction(hack_targetFunc, 0);

#ifdef PASS_EXPECTED_ACCESS_INFO
    RTN_AddInstrumentFunction(hack_setAccessCnt, 0);
#endif

    PIN_AddFiniFunction(Fini, 0);

    PIN_StartProgram();  
    
    fclose(outfile);
    fclose(resfile);
    fclose(descriptFile);
    
    return 0;
}

//-------util-----------------

void print_globals(FILE* file){
    fprintf(file, "all global variables\n");
    for(int i=0;i<cnt_globals;i++){
        fprintf(file, "0x%lx %d  %s\n", globals[i].addr, globals[i].size, globals[i].name);
    }
    fprintf(file,"\n");
}