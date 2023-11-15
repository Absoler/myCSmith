/*
 * This is a RANDOMLY GENERATED PROGRAM.
 *
 * Generator: csmith 2.3.0
 * Git version: aa32d19
 * Options:   --copyPropagation --no-safe-math --no-bitfields --no-volatiles --probability-configuration ./prob.txt -o runtime/output11.c
 * Seed:      2497236573
 */

typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef signed short int int16_t;
typedef unsigned short int uint16_t;
typedef signed int int32_t;
typedef unsigned int uint32_t;
typedef signed long int int64_t;
typedef unsigned long int uint64_t;


#include<stdlib.h>
#include<signal.h>
#include<stdio.h>
#include<string.h>
/* --- Struct/Union Declarations --- */


struct S0 {
   int64_t  f0;
   uint32_t  f1;
};


union U1 {
   struct S0  f0;
};

/* --- GLOBAL VARIABLES --- */
union U1 us[2];
struct S0 *g_73 = &us[1].f0;
struct S0 g_242 = {0xD82274A53ECC2041LL,0UL};


/* --- FORWARD DECLARATIONS --- */
union U1  func_1(void);

int  func_8(union U1  p_10);

struct S0 s;
union U1 us[2];
int ret = 1;
int n1, n2, n3, n4;
int f1, f2, f3;
int *p = &f1;
__attribute__((noinline)) union U1 func_1() {
    struct S0 *d = &s;
    if (func_8(us[1]) != f1)
        *d = *g_73;     // use
    if (f2 && *p && f3) {
        n3 = n4 * ( n2 < n1) ;
    }
    return us[0];
}

int func_8() {
    *g_73 = g_242;  // clone
    return ret;
}


/*
func_8():
/home/csmith_clang/examples/output2.c:65
  401190:	48 8b 05 c9 2e 00 00 	mov    0x2ec9(%rip),%rax        # 404060 <g_73>
  401197:	0f 10 05 ca 2e 00 00 	movups 0x2eca(%rip),%xmm0        # 404068 <g_242>
  40119e:	0f 11 00             	movups %xmm0,(%rax)
/home/csmith_clang/examples/output2.c:66
  4011a1:	8b 05 d1 2e 00 00    	mov    0x2ed1(%rip),%eax        # 404078 <ret>
func_1():
/home/csmith_clang/examples/output2.c:56
  4011a7:	3b 05 13 2f 00 00    	cmp    0x2f13(%rip),%eax        # 4040c0 <f1>
  4011ad:	74 0e                	je     4011bd <func_1+0x2d>
/home/csmith_clang/examples/output2.c:57
  4011af:	0f 10 05 b2 2e 00 00 	movups 0x2eb2(%rip),%xmm0        # 404068 <g_242>
  4011b6:	0f 11 05 0b 2f 00 00 	movups %xmm0,0x2f0b(%rip)        # 4040c8 <s>
*/

unsigned long side=0;
#define MAX_VAR_NUM 1000
struct VarInfo{
	unsigned long addr;
	unsigned int size;
	char name[10];
}varInfos[1000];
int global_cnt=0;
#define MAX_ACCESS_NUM 2000
struct AccessInfo{
    unsigned long addr;
    unsigned int length;
    int cnt;
}accesses[MAX_ACCESS_NUM];
int access_cnt=0;
void __attribute__ ((noinline)) getInfo(unsigned long varInfosAddr, int varCnt, unsigned long accessesAddr, int accessCnt){
    side=(side+(varInfosAddr+varCnt)%1000 + (accessesAddr+accessCnt)%1000)%1000;
}
void __attribute__ ((noinline)) setInfo(unsigned long addr, unsigned int size, char* name){
	varInfos[global_cnt].addr=addr;
	varInfos[global_cnt].size=size;
	strcpy(varInfos[global_cnt].name,name);
	global_cnt++;
  side=(side+size)%1000;
}
void __attribute__ ((noinline)) setReadCnt(unsigned long addr, unsigned int length, int cnt){
    accesses[access_cnt].addr=addr;
    accesses[access_cnt].length = length;
    accesses[access_cnt].cnt =cnt;
    access_cnt++;
}
void handleSignal(int val){
	if(val==SIGTERM){
        printf("timeout\n");
        FILE* resfile=fopen("timeout.out","a");
        fclose(resfile);
        exit(0);
	}
}


/* ---------------------------------------- */
int main (int argc, char* argv[])
{
    int i, j;
	signal(SIGTERM, handleSignal);

	side=(side+(unsigned long)func_1%1000)%1000;
	side=(side+(unsigned long)func_8%1000)%1000;
	side=(side+(unsigned long)getInfo%1000)%1000;
	side=(side+(unsigned long)setInfo%1000)%1000;
	side=(side+(unsigned long)setReadCnt%1000)%1000;

    setInfo((unsigned long)(&g_242),sizeof(g_242),"g_242");

    setReadCnt((unsigned long)(&g_242), sizeof(g_242), 1);
    
    getInfo((unsigned long)varInfos, global_cnt, (unsigned long)accesses, access_cnt);
    func_1();

    side=(side+(unsigned long)&g_242)%1000;
    

	printf("%d\n",side);
    return 0;
}
