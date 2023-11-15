// -O1 option must
int f = 1;
int g1, g2, arr[10];

int func_1() {
    int a = 4;
    int *b = &g1;
    if (f) {
        arr[0] = g2;
        int *c = &a;
        *c = 1;
    }
    *b &= (a > 9) - 5;
    return a;
}

int main() {
    func_1();
}


/*
redundant preload of g1

func_1():
/home/csmith_clang/examples/output2.c:12
  401102:	8b 05 84 2f 00 00    	mov    0x2f84(%rip),%eax        # 40408c <g1>
/home/csmith_clang/examples/output2.c:5
  401108:	ba 04 00 00 00       	mov    $0x4,%edx
/home/csmith_clang/examples/output2.c:7
  40110d:	83 3d 14 2f 00 00 00 	cmpl   $0x0,0x2f14(%rip)        # 404028 <f>
  401114:	74 17                	je     40112d <func_1+0x2b>
/home/csmith_clang/examples/output2.c:8
  401116:	8b 05 6c 2f 00 00    	mov    0x2f6c(%rip),%eax        # 404088 <g2>
  40111c:	89 05 3e 2f 00 00    	mov    %eax,0x2f3e(%rip)        # 404060 <arr>
/home/csmith_clang/examples/output2.c:12
  401122:	8b 05 64 2f 00 00    	mov    0x2f64(%rip),%eax        # 40408c <g1>
/home/csmith_clang/examples/output2.c:10
  401128:	ba 01 00 00 00       	mov    $0x1,%edx
/home/csmith_clang/examples/output2.c:12 (discriminator 2)
  40112d:	83 e0 fb             	and    $0xfffffffb,%eax
  401130:	89 05 56 2f 00 00    	mov    %eax,0x2f56(%rip)        # 40408c <g1>
/home/csmith_clang/examples/output2.c:14 (discriminator 2)
  401136:	89 d0                	mov    %edx,%eax
  401138:	c3                   	ret
 */