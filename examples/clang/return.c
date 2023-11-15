/*
 * This is a RANDOMLY GENERATED PROGRAM.
 *
 * Generator: csmith 2.3.0
 * Git version: aa32d19
 * Options:   --copyPropagation --no-safe-math --no-bitfields --no-volatiles --probability-configuration ./prob.txt -o runtime/output9.c
 * Seed:      1666252725
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
   int  f0;
   long long  f1;
   int  f2;
   long long  f3;
   unsigned  f4;
   unsigned char  f5;
};

/* --- GLOBAL VARIABLES --- */
int g_4 = 1L;
int g_20[5] = {0x70E81DE9L,0x70E81DE9L,0L,0x70E81DE9L,0x70E81DE9L};
struct S0 g_50 = {0x2005908CL,0x2CC25678016B841ALL,0x9EC2B408L,4L,9UL,0x6DL};
struct S0 *g_93 = &g_50;
struct S0 p = {0xDAA861DAL,0xFA16DD20EC94203ALL,0x05008DB5L,0x3B29BC1B99DA1B10LL,0xFE3058E3L,1UL};
int *g_61 = &g_20[3];
int *g_146 = &g_20[4];

/* --- FORWARD DECLARATIONS --- */

int n1, n2;
int *p1 = &n1, *p2 = &g_20[3];
struct S0 s;
struct S0 *ps;

__attribute_noinline__ struct S0 func_5(int *arg) {
  struct S0 c = {3, 1, 1, 4, 0, 7};
  if (*arg) {
    int *d = g_146, *f = g_146;
    struct S0 e = {7, 8, 5, 1, 3, 1};
    if (g_61 != arg) {
      *d = n1;
      *f = p1[0];
      *g_93 = e;
    } else {
      *g_93 = p;   // clone g_124 to (g_93)
      n1 = 0;
      return *g_93;     // compile to return g_124 directly
    }
    *g_93 = e;
    p.f0 = n2;
    if (p.f0)
      *g_93 = c;
  }
  return s;
}

/*

/home/csmith_clang/examples/output2.c:60
  4012a5:	48 8b 0d 04 2e 00 00 	mov    0x2e04(%rip),%rcx        # 4040b0 <g_93>
  4012ac:	48 8b 15 25 2e 00 00 	mov    0x2e25(%rip),%rdx        # 4040d8 <g_124+0x20>
  4012b3:	48 89 51 20          	mov    %rdx,0x20(%rcx)
  4012b7:	0f 10 05 0a 2e 00 00 	movups 0x2e0a(%rip),%xmm0        # 4040c8 <g_124+0x10>
  4012be:	0f 11 41 10          	movups %xmm0,0x10(%rcx)
  4012c2:	0f 10 05 ef 2d 00 00 	movups 0x2def(%rip),%xmm0        # 4040b8 <g_124>
  4012c9:	0f 11 01             	movups %xmm0,(%rcx)
/home/csmith_clang/examples/output2.c:61
  4012cc:	c7 05 3a 2e 00 00 00 	movl   $0x0,0x2e3a(%rip)        # 404110 <n1>
  4012d3:	00 00 00 
/home/csmith_clang/examples/output2.c:62
  4012d6:	48 8b 0d fb 2d 00 00 	mov    0x2dfb(%rip),%rcx        # 4040d8 <g_124+0x20>
  4012dd:	48 89 48 20          	mov    %rcx,0x20(%rax)
  4012e1:	0f 10 05 e0 2d 00 00 	movups 0x2de0(%rip),%xmm0        # 4040c8 <g_124+0x10>
  4012e8:	0f 11 40 10          	movups %xmm0,0x10(%rax)
  4012ec:	0f 10 05 c5 2d 00 00 	movups 0x2dc5(%rip),%xmm0        # 4040b8 <g_124>
  4012f3:	0f 11 00             	movups %xmm0,(%rax)

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
    int i, j, k;
	signal(SIGTERM, handleSignal);

	side=(side+(unsigned long)func_5%1000)%1000;
	
	side=(side+(unsigned long)getInfo%1000)%1000;
	side=(side+(unsigned long)setInfo%1000)%1000;
	side=(side+(unsigned long)setReadCnt%1000)%1000;

    setInfo((unsigned long)(&p),sizeof(p),"g_124");
    setInfo((unsigned long)(&g_61),sizeof(g_61),"g_61");
 

    setReadCnt((unsigned long)(&g_61), sizeof(g_61), 38);
    setReadCnt((unsigned long)(&p), sizeof(p), 1);

    getInfo((unsigned long)varInfos, global_cnt, (unsigned long)accesses, access_cnt);
    func_5(p2);

    side=(side+(unsigned long)&p)%1000;
    side=(side+(unsigned long)g_61)%1000;


	printf("%d\n",side);
    return 0;
}
