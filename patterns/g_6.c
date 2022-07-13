
#include<stdio.h>
int g_6[2] = {0x67D50F82L,0x67D50F82L};
int g_10 = 0L;

int func_1(void)
{ 
    int *l_9 = &g_10;
    (*l_9) = (g_6[0]);
    if (g_6[1])
    {
        return g_10;
    }
    return 1;
}

int main(){
    printf("%d\n", func_1());
}

