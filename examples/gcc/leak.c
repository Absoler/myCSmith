#include<stdio.h>
#pragma pack(2)
struct S2 {
   int  f0;
   short  f1;
};
struct S2 g_32[2][2][1] = {{{{0x200CC90FL,0x27C9L}},{{0x9A802726L,0x125BL}}},{{{0xE23F1199L,-4L}},{{4294967292UL,0xD72EL}}}};

void f1(struct S2 p1) {
    p1.f1 += 1;
    int* p = (int *)&p1;
    printf("%x\n", p[1]);
}

int main(){
    scanf("%d", &g_32[0][1][0].f0);
    f1(g_32[0][0][0]);
}

/*

/home/csmith_clang/examples/output2.c:17
  401065:	48 8b 3d d4 2f 00 00 	mov    0x2fd4(%rip),%rdi        # 404040 <g_32>
  40106c:	e8 ff 00 00 00       	call   401170 <f1>


it just load first 8 bytes of g_32 directly as argument, 
thus in f1(), the first 2 bytes of g_32[0][1][0] can be read.
For example, when executing this program and input 1, 
then the output would be 127ca, which could lead to vulnerabilities

 */