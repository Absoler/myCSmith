#include<stdio.h>
#pragma pack(1)
struct S2 {
   int  f0;
   short  f1;
};
struct S2 g_32[2][2][1] = {{{{0x200CC90FL,0x27C9L}},{{0x9A802726L,0x125BL}}},{{{0xE23F1199L,-4L}},{{4294967292UL,0xD72EL}}}};

// FUNCTIONS
void f1(struct S2 p1){
    
    p1.f1+=1;
    int* p=(void*)&p1;
    int a=0;
    printf("%x\n",p[1]);
}

int main(){
    scanf("%d", &g_32[0][1][0].f0);
    f1(g_32[0][0][0]);
}