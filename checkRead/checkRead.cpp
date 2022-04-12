#include "pin.h"
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

#define MAX_GLOBAL_VAR_NUM 1000

string targetFuncPrefix;
FILE* outfile;
FILE* resfile;
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
void print_reads(FILE* file);
void print_globals(FILE* file);

//---------get global info------------
VOID record_var(ADDRINT _addr, ADDRINT cnt){
    // printf("pin info-addr: %lu    cnt: %ld\n",_addr,cnt);
    printf("num of global vars  %ld\n",cnt);
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

#ifdef VERSION_1
struct Read{	//record a read operation
	ADDRINT start, ins_ptr; 
    UINT32 length;
	BOOL isGlobal;
    char name[10];
};
int read_limit=10000;
vector<Read> reads(read_limit);

bool comp_Read(const Read& read1, const Read& read2){
    if(read1.isGlobal&&(!read2.isGlobal)){
        return true;
    }else if((!read1.isGlobal)&&read2.isGlobal){
        return false;
    }else{
    	return read1.start<read2.start;
    }
}
int cnt_reads=0;

void print_reads(FILE* file){
    fprintf(file, "all read memory\n");
    for(int i=0;i<cnt_reads;i++){
        fprintf(file, "0x%lx %d %s\n", reads[i].start, reads[i].length, (reads[i].isGlobal?"global":"local"));
    }
    fprintf(file,"\n");
}

map<ADDRINT, string> disasMap;

//----------record read info-----------
VOID record_Read(ADDRINT ip, ADDRINT _start, UINT32 _length) { 
    if(cnt_reads>=read_limit-1){
        if(read_limit>=5e5){
            printf("too many reads!(>500000)");
            exit(1);
        }
        printf("%d reads!\n",read_limit);
        read_limit+=10000;
        reads.resize(read_limit);
    }
    reads.push_back(Read());
    reads[cnt_reads].start=_start;
    reads[cnt_reads].length=_length;
    reads[cnt_reads].ins_ptr= ip;
    reads[cnt_reads].isGlobal=false;
    cnt_reads++;
}

VOID hack_targetFunc(RTN rtn, VOID* v){
    RTN_Open(rtn);
    // printf("%s\n",RTN_Name(rtn).substr(0,targetFuncPrefix.length()).c_str());
    if(RTN_Name(rtn).substr(0,targetFuncPrefix.length())==targetFuncPrefix){
        
		printf("%s\n",RTN_Name(rtn).c_str());
        for (INS ins = RTN_InsHead(rtn); INS_Valid(ins); ins = INS_Next(ins)){
            if (INS_IsMemoryRead(ins)){
                disasMap[INS_Address(ins)] = INS_Disassemble(ins);
                INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)record_Read, IARG_INST_PTR, IARG_MEMORYREAD_EA, IARG_MEMORYREAD_SIZE, IARG_END);
            }
        }    
    }
    RTN_Close(rtn);
}

//----------analysis--------
VOID Fini(INT32 code, VOID* val){
	sort(globals, globals+cnt_globals, comp_Var);
    //identify all global reads
    for(int i=0;i<cnt_reads;i++){
        // globals[id] 's address is the maximum less than or equal to reads[i].start
        int id=upper_bound(globals, globals+cnt_globals, (Var){reads[i].start, 0}, comp_Var)-globals-1;
        if(id>=0&&id<cnt_globals){
            // mark global read
            reads[i].isGlobal = (reads[i].start<globals[id].addr+globals[id].size && reads[i].start+reads[i].length<=globals[id].addr+globals[id].size);
            if(reads[i].isGlobal){
                strcpy(reads[i].name,globals[id].name);
            }
        }
    }
    print_globals(outfile);
    fprintf(outfile,"\n\noverlap:\n");
    // check whether reads overlap
    /* Supports detecting multiple executions of the same statement, */
    // sort(reads, reads+cnt_reads, comp_Read);
    // bool overlap=false;
    // for(int i=0; i<cnt_reads-1; i++){
    //     if(reads[i+1].isGlobal){
    //         overlap=(reads[i].start+reads[i].length>reads[i+1].start&&reads[i+1].isGlobal);  //next read occur in current read range
    //         if(overlap){
    //             fprintf(outfile, "0x%lx: %s 0x%lx: %s len:%u    |    0x%lx: %s 0x%lx: %s len:%u\n", reads[i].ins_ptr, disasMap[reads[i].ins_ptr].c_str(), reads[i].start, reads[i].name, reads[i].length,
    //                                                                                                 reads[i+1].ins_ptr, disasMap[reads[i+1].ins_ptr].c_str(), reads[i+1].start, reads[i+1].name, reads[i+1].length);
    //         }                                                                            
    //     }
    // }
    bool overlap=false;
    for(int i=0;i<cnt_reads-1;i++){
        if(!reads[i].isGlobal){
            continue;
        }
        for(int j=i+1;j<cnt_reads;j++){
            if(!reads[j].isGlobal){
                continue;
            }
            if(reads[i].ins_ptr==reads[j].ins_ptr){
                //only consider overlap from different instructions
                continue;
            }
            if(reads[i].start<reads[j].start){
                //overlap occurs when front read spans back read
                overlap=(reads[i].start+reads[i].length>reads[j].start);
            }else if(reads[i].start>reads[j].start){
                overlap=(reads[j].start+reads[j].length>reads[i].start);
            }else{
                overlap=true;
            }
            if(overlap){
                fail=true;
                fprintf(outfile, "0x%lx: %s 0x%lx: %s len:%u    |    0x%lx: %s 0x%lx: %s len:%u\n", reads[i].ins_ptr, disasMap[reads[i].ins_ptr].c_str(), reads[i].start, reads[i].name, reads[i].length,\
                                                                                                    reads[j].ins_ptr, disasMap[reads[j].ins_ptr].c_str(), reads[j].start, reads[j].name, reads[j].length);
            }
        }
    }
    fprintf(resfile,(fail?"1":"0"));
    print_reads(stdout);
}

#endif

#ifdef VERSION_2
typedef pair<ADDRINT, uint32_t> Read;

// a read is represented as <addr, len>
// PROBLEM: under this strict definition, if compiler split a read in half or combine two reads, then we cant't identifiy it 
map<Read, int> expect_counter;    // contain all global vars read, and their expected read times (up-limit for now)
map<Read, int> actual_counter;    // record actual read times of each reads 
set<Read> unexpectedReads;      
set<Read> actual_clusters[MAX_GLOBAL_VAR_NUM];
set<Read> expect_clusters[MAX_GLOBAL_VAR_NUM];
map<Read, set<ADDRINT>> instOfRead;    // record all insts addrs a read occur
map<Read, int> varOfRead;              // record which var the read belongs to (record index)
map<Read, bool> isForVars;    // record whether a read is forVar, maybe not necessary now, 'cause we combine two situation
map<Read, bool> isGlobals;    // record whether a read located in global, not necessary too.

map<ADDRINT, string> disasMap;  // store instruction text

inline int getInd(const Read& read){
    return upper_bound(globals, globals+cnt_globals, (Var){read.first, read.second}, comp_Var)-globals-1;
}

inline bool partOf(const Read& read, const Var& var){
    return read.first<var.addr+var.size && read.first+read.second<=var.addr+var.size;
}
inline bool partOf(const Read& read1, const Read& read2){
    return read1.first<read2.first+read2.second && read1.first+read1.second<=read2.first+read2.second;
}

bool isGlobal(const Read &read){
    // ADDRINT start=read.first;
    // unsigned len=read.second;
    
    int id=getInd(read);
    bool res=false;
    if(id>=0 && id<cnt_globals){
        res = partOf(read, globals[id]);
    }

    return res;
}

set<Read> findContainers(const Read& target, const set<Read>& readset){
    set<Read> res;
    for(auto read:readset){
        if(partOf(target, read)){
            res.insert(read);
        }
    }
    return res;
}


//----------get vars' limit--------------
VOID record_ReadCnt(ADDRINT addr, int len, int cnt){
    Read read=std::make_pair<ADDRINT, int>(addr, len);
    expect_counter[read]=cnt;
}
VOID hack_setReadCnt(RTN rtn, VOID* v){
    RTN_Open(rtn);
    if(RTN_Name(rtn)=="setReadCnt"){
        RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)record_ReadCnt,
            IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
            IARG_FUNCARG_ENTRYPOINT_VALUE, 1,
            IARG_FUNCARG_ENTRYPOINT_VALUE, 2, IARG_END);
    }
    RTN_Close(rtn);
}

//----------record read info-----------
VOID record_Read(ADDRINT ip, ADDRINT start, UINT32 len) { 
    Read read=std::make_pair(start, len);
    if(isGlobal(read)){
        
        actual_counter[read]++;
        if(instOfRead[read].find(ip)==instOfRead[read].end()){
            // we don't know this inst load this content before
            instOfRead[read].insert(ip);
        }
    }
}

VOID hack_targetFunc(RTN rtn, VOID* v){
    RTN_Open(rtn);
    if(RTN_Name(rtn).substr(0,targetFuncPrefix.length())==targetFuncPrefix){
        for (INS ins = RTN_InsHead(rtn); INS_Valid(ins); ins = INS_Next(ins)){
            if (INS_IsMemoryRead(ins)){
                disasMap[INS_Address(ins)] = INS_Disassemble(ins);
                INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)record_Read, IARG_INST_PTR, IARG_MEMORYREAD_EA, IARG_MEMORYREAD_SIZE, IARG_END);
            }
        }    
    }
    RTN_Close(rtn);
}

//----------analysis--------
VOID Fini(INT32 code, VOID* val){
    
    print_globals(outfile);

    // build relations between global vars and global reads
    for(auto p:actual_counter){
        Read read=p.first;
        int id=getInd(read);
        assert(partOf(read, globals[id]));
        varOfRead[read]=id;
        actual_clusters[id].insert(read);
    }
    for(auto p:expect_counter){
        Read read=p.first;
        int id=getInd(read);
        assert(partOf(read, globals[id]));
        varOfRead[read]=id;
        expect_clusters[id].insert(read);
    }

    fprintf(outfile,"\n\nproblem of too many reads:\n");
    for(int id=0;id<cnt_globals;id++){
        if(actual_clusters[id].size()>0){
            for(Read read:actual_clusters[id]){
                set<Read> actual_set=findContainers(read, actual_clusters[id]);
                set<Read> expect_set=findContainers(read, expect_clusters[id]);
                if(expect_set.empty()){
                    unexpectedReads.insert(read);
                    continue;
                }
                int actual_cnt=0, expect_cnt=0;
                for(Read read:actual_set){
                    actual_cnt+=actual_counter[read]; 
                }
                for(Read read:expect_set){
                    expect_cnt+=expect_counter[read];
                }
                if(actual_cnt>expect_cnt){
                    fprintf(outfile, "%s: start at 0x%lx of len = %d\n", globals[varOfRead[read]].name, read.first, read.second);
                    fprintf(outfile, "expected %d but read %d\n", expect_cnt, actual_cnt);
                    for(ADDRINT ip: instOfRead[read]){
                        fprintf(outfile, "  %s\n", disasMap[ip].c_str());
                    }
                    fprintf(outfile, "\n");
                    fail=true;
                }
            }
        }
    }
    
    
    fprintf(outfile, "problem of unexpected reads\n");
    for(Read read: unexpectedReads){
        fprintf(outfile, "%s: start at 0x%lx of len = %d\n", globals[varOfRead[read]].name, read.first, read.second);
        for(ADDRINT addr:instOfRead[read]){
            fprintf(outfile, "  %s\n", disasMap[addr].c_str());
        }
        fprintf(outfile, "\n");
        fail=true;
    }
    
    fprintf(resfile,(fail?"1":"0"));
}

#endif

INT32 Usage(){

    return -1;
}


int main(int argc, char* argv[]){
    targetFuncPrefix=argv[argc-1];
    PIN_InitSymbols();
    outfile=fopen("checkRead.out", "w");
    resfile=fopen("result.out", "w");
    if(PIN_Init(argc, argv)) return Usage();

    RTN_AddInstrumentFunction(hack_getInfo, 0);
    RTN_AddInstrumentFunction(hack_targetFunc, 0);
    RTN_AddInstrumentFunction(hack_setReadCnt, 0);
    
    PIN_AddFiniFunction(Fini, 0);

    PIN_StartProgram();  
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