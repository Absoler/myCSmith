#include "pin.h"
#include<string>
#include<cstdio>
#include<algorithm>
#include<map>
#include<cstring>
#include<vector>
using std::string;
using std::map;
using std::upper_bound;
using std::sort;
using std::vector;

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
int cnt_globals=0, cnt_reads=0;
map<ADDRINT, string> disasMap;


//---------get global info------------
VOID record_var(ADDRINT _addr, ADDRINT cnt){
    // printf("pin info-addr: %lu    cnt: %ld\n",_addr,cnt);
    printf("num of global vars%ld\n",cnt);
    if(cnt>MAX_GLOBAL_VAR_NUM){
        printf("too many global variables!\n");
        exit(1);
    }
    PIN_SafeCopy(globals,Addrint2VoidStar(_addr),sizeof(Var)*cnt);
    cnt_globals=cnt;
    // for(int i=0;i<cnt_globals;i++){
    //     printf(" %s\n",globals[i].name);
    // }
    // printf("%lu\n",res);
	// globals[cnt_globals++] = (Var){_addr, _size};
}
// VOID testName(ADDRINT _addr, ADDRINT _size){
//     printf("2\n");
//     char name[10];
//     name[_size]=0;
//     PIN_SafeCopy(name,Addrint2VoidStar(_addr),_size);
//     printf("getname %s\n",name);
// }
VOID hack_getInfo(RTN rtn, VOID* v){
	RTN_Open(rtn);
	if(RTN_Name(rtn)=="getInfo"){
        printf("1\n");
		RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)record_var,
			IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
			IARG_FUNCARG_ENTRYPOINT_VALUE, 1, IARG_END);
	}
	RTN_Close(rtn);
}

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


//-------util----------------
void print_reads(FILE* file);
void print_globals(FILE* file);

//----------analysis--------
VOID Fini(INT32 code, VOID* val){
	sort(globals, globals+cnt_globals, comp_Var);
    for(int i=0;i<cnt_reads;i++){
        // globals[id] 's address is the maximum less than or equal to reads[i].start
        int id=upper_bound(globals, globals+cnt_globals, (Var){reads[i].start, 0}, comp_Var)-globals-1;
        if(id>=0&&id<cnt_globals){
            // mark global read
            //printf("%lx %d\n",reads[i].start, id);
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
	// print_globals(stdout);
    // fprintf(outfile, "#eof\n");
}

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

    
    PIN_AddFiniFunction(Fini, 0);

    PIN_StartProgram();  
    return 0;
}

//-------util-----------------
void print_reads(FILE* file){
    fprintf(file, "all read memory\n");
    for(int i=0;i<cnt_reads;i++){
        fprintf(file, "0x%lx %d %s\n", reads[i].start, reads[i].length, (reads[i].isGlobal?"global":"local"));
    }
    fprintf(file,"\n");
}
void print_globals(FILE* file){
    fprintf(file, "all global variables\n");
    for(int i=0;i<cnt_globals;i++){
        fprintf(file, "0x%lx %d  %s\n", globals[i].addr, globals[i].size, globals[i].name);
    }
    fprintf(file,"\n");
}