/*
 * This is a RANDOMLY GENERATED PROGRAM.
 *
 * Generator: csmith 2.3.0
 * Git version: aa32d19
 * Options:   --copyPropagation --no-safe-math --no-bitfields --no-volatiles --probability-configuration ./prob.txt -o runtime/output6.c
 * Seed:      2549617326
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
   short  f0;
   char  f1;
   int  f2;
   int  f3;
   int  f4;
   int  f5;
};


struct S0 g_479 = {9L,0x25L,0x3C047BBEL,-1L,0xF9DF4619L,0x6D8CADDFL};


int f1 = 1, f2;
int n1, n2;
int *p = &n1;
struct S0 s, ret;
__attribute__((noinline)) struct S0 func_9(int h, struct S0 i) {
    if (f1)
        i = g_479;  // here
    else {
        return ret;
    }
    s = i;   // replace i with g_479
    if (i.f5)
        *p ^= n2;
    return ret;
}

/*
0000000000401190 <func_9>:
func_9():
/home/csmith_clang/examples/output2.c:42
  401190:	48 89 f8             	mov    %rdi,%rax
/home/csmith_clang/examples/output2.c:43
  401193:	83 3d da 2e 00 00 00 	cmpl   $0x0,0x2eda(%rip)        # 404074 <f1>
  40119a:	74 47                	je     4011e3 <func_9+0x53>
  40119c:	48 8d 4c 24 08       	lea    0x8(%rsp),%rcx
/home/csmith_clang/examples/output2.c:44
  4011a1:	8b 15 c9 2e 00 00    	mov    0x2ec9(%rip),%edx        # 404070 <g_479+0x10>
  4011a7:	89 51 10             	mov    %edx,0x10(%rcx)
  4011aa:	0f 10 05 af 2e 00 00 	movups 0x2eaf(%rip),%xmm0        # 404060 <g_479>
  4011b1:	0f 11 01             	movups %xmm0,(%rcx)
/home/csmith_clang/examples/output2.c:48
  4011b4:	8b 15 b6 2e 00 00    	mov    0x2eb6(%rip),%edx        # 404070 <g_479+0x10>
  4011ba:	89 15 f8 2e 00 00    	mov    %edx,0x2ef8(%rip)        # 4040b8 <s+0x10>
  4011c0:	0f 10 05 99 2e 00 00 	movups 0x2e99(%rip),%xmm0        # 404060 <g_479>
  4011c7:	0f 11 05 da 2e 00 00 	movups %xmm0,0x2eda(%rip)        # 4040a8 <s>
/home/csmith_clang/examples/output2.c:49
  4011ce:	83 79 10 00          	cmpl   $0x0,0x10(%rcx)
  4011d2:	74 0f                	je     4011e3 <func_9+0x53>
/home/csmith_clang/examples/output2.c:50
  4011d4:	8b 0d e2 2e 00 00    	mov    0x2ee2(%rip),%ecx        # 4040bc <n2>
  4011da:	48 8b 15 97 2e 00 00 	mov    0x2e97(%rip),%rdx        # 404078 <p>
  4011e1:	31 0a                	xor    %ecx,(%rdx)
  4011e3:	8b 0d bb 2e 00 00    	mov    0x2ebb(%rip),%ecx        # 4040a4 <ret+0x10>
  4011e9:	89 48 10             	mov    %ecx,0x10(%rax)
  4011ec:	0f 10 05 a1 2e 00 00 	movups 0x2ea1(%rip),%xmm0        # 404094 <ret>
  4011f3:	0f 11 00             	movups %xmm0,(%rax)
/home/csmith_clang/examples/output2.c:52
  4011f6:	c3                   	ret
  4011f7:	66 0f 1f 84 00 00 00 	nopw   0x0(%rax,%rax,1)

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

	side=(side+(unsigned long)func_9%1000)%1000;
	
	side=(side+(unsigned long)getInfo%1000)%1000;
	side=(side+(unsigned long)setInfo%1000)%1000;
	side=(side+(unsigned long)setReadCnt%1000)%1000;
    
    setInfo((unsigned long)(&g_479),sizeof(g_479),"g_479");
    
   
    setReadCnt((unsigned long)(&g_479), sizeof(g_479), 1);
    
getInfo((unsigned long)varInfos, global_cnt, (unsigned long)accesses, access_cnt);
    func_9(0, s);
    

    
    side=(side+(unsigned long)&g_479)%1000;
    side=(side+(unsigned long)n1)%1000;
    side=(side+(unsigned long)n2)%1000;
    side=(side+(unsigned long)s.f0)%1000;

	printf("%d\n",side);
    return 0;
}
