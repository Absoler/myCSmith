#include <stdio.h>
struct S0 {
   int  f0;
   char  f1;
   unsigned char f2;
};
struct S0 g_52 = {1,2,3};
int g;

__attribute__((noinline)) int func_1() {
  	struct S0 b = g_52;
  	int d[1];
    d[0] = 1;
    if (b.f0)
  	    d[0] &= ((--g && b.f2) || b.f1);
	return d[0];
}

int main(){
    func_1();
}

/*

func_1():
/home/csmith_clang/examples/output2.c:13
  401250:	b8 01 00 00 00       	mov    $0x1,%eax
/home/csmith_clang/examples/output2.c:11
  401255:	48 8b 15 44 5e 00 00 	mov    0x5e44(%rip),%rdx        # 4070a0 <g_52>
  40125c:	48 89 54 24 f8       	mov    %rdx,-0x8(%rsp)
/home/csmith_clang/examples/output2.c:14
  401261:	85 d2                	test   %edx,%edx
  401263:	74 1a                	je     40127f <func_1+0x2f>
/home/csmith_clang/examples/output2.c:15
  401265:	ff 0d 45 69 00 00    	decl   0x6945(%rip)        # 407bb0 <g>
  40126b:	74 09                	je     401276 <func_1+0x26>
  40126d:	80 3d 31 5e 00 00 00 	cmpb   $0x0,0x5e31(%rip)        # 4070a5 <g_52+0x5> b.f2 is replaced
  401274:	75 09                	jne    40127f <func_1+0x2f>
  401276:	80 7c 24 fc 00       	cmpb   $0x0,-0x4(%rsp)
  40127b:	75 02                	jne    40127f <func_1+0x2f>
  40127d:	33 c0                	xor    %eax,%eax
/home/csmith_clang/examples/output2.c:16
  40127f:	c3                   	ret

 */