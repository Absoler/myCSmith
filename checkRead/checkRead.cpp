#include "pin.h"
#include<string>
#include<cstdio>
#include<algorithm>
#include<map>
#include<cstring>
using std::string;
using std::map;
using std::upper_bound;
using std::sort;

#define MAX_GLOBAL_VAR_NUM 1000
#define MAX_MEMORY_READ_NUM 10000
string targetFuncPrefix;
FILE* outfile;


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
}reads[MAX_MEMORY_READ_NUM];
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
    if(cnt_reads>=MAX_MEMORY_READ_NUM){
        printf("too many reads!\n");
        exit(1);
    }
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
    sort(reads, reads+cnt_reads, comp_Read);
    bool overlap=false;
    for(int i=0; i<cnt_reads-1; i++){
        if(reads[i+1].isGlobal){
            overlap=(reads[i].start+reads[i].length>reads[i+1].start&&reads[i+1].isGlobal);  //next read occur in current read range
            if(overlap){
                fprintf(outfile, "0x%lx: %s 0x%lx: %s len:%u    |    0x%lx: %s 0x%lx: %s len:%u\n", reads[i].ins_ptr, disasMap[reads[i].ins_ptr].c_str(), reads[i].start, reads[i].name, reads[i].length,\
                                                                                                    reads[i+1].ins_ptr, disasMap[reads[i+1].ins_ptr].c_str(), reads[i+1].start, reads[i+1].name, reads[i+1].length);
            }                                                                            
        }
    }
    
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
        fprintf(file, "0x%lx %d\n", globals[i].addr, globals[i].size);
    }
    fprintf(file,"\n");
}