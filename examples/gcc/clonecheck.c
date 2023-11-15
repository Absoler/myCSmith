struct S6 {
   short  f0;
   char  f1;
   int  f2;
};

struct S6 g_36 = {2UL, 4L ,0x32B3D10AL};
int n1 = (-1L), n2;

int func_30() {
    struct S6 f[4][7];
    f[1][6] = g_36;
    n2 = 1;
    if (f[1][6].f0 || f[1][6].f1)
        n1 = f[1][6].f1;
    return n1;
}

int main(){
    func_30();
}

/*
 /home/csmith_clang/examples/output2.c:12
  401134:	0f be 05 f7 2e 00 00 	movsbl 0x2ef7(%rip),%eax        # 404032 <g_36+0x2>
/home/csmith_clang/examples/output2.c:13
  40113b:	c7 05 f7 2e 00 00 01 	movl   $0x1,0x2ef7(%rip)        # 40403c <n2>
  401142:	00 00 00 
/home/csmith_clang/examples/output2.c:14
  401145:	f7 05 e1 2e 00 00 ff 	testl  $0xffffff,0x2ee1(%rip)        # 404030 <g_36>
  40114c:	ff ff 00 
  40114f:	75 0f                	jne    401160 <func_30+0x30>
  
 */