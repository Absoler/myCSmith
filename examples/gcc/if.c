int f;
int a, b;
int func() {
    if (f) {
        a = 4;
        b = 9;
    } else {
        a = 8;
        b = 27;
    }
    return a + b;
}

int main() {
    func();
}

/*
 func():
/home/csmith_clang/examples/output2.c:4
  401120:	83 3d 0d 2f 00 00 01 	cmpl   $0x1,0x2f0d(%rip)        # 404034 <f>
  401127:	19 c0                	sbb    %eax,%eax
  401129:	83 e0 16             	and    $0x16,%eax
  40112c:	83 c0 0d             	add    $0xd,%eax
  40112f:	83 3d fe 2e 00 00 01 	cmpl   $0x1,0x2efe(%rip)        # 404034 <f>
  401136:	19 c9                	sbb    %ecx,%ecx
  401138:	83 e1 12             	and    $0x12,%ecx
  40113b:	83 c1 09             	add    $0x9,%ecx
  40113e:	83 3d ef 2e 00 00 01 	cmpl   $0x1,0x2eef(%rip)        # 404034 <f>
  401145:	19 d2                	sbb    %edx,%edx
  401147:	89 0d df 2e 00 00    	mov    %ecx,0x2edf(%rip)        # 40402c <b>
  40114d:	83 e2 04             	and    $0x4,%edx
  401150:	83 c2 04             	add    $0x4,%edx
  401153:	89 15 d7 2e 00 00    	mov    %edx,0x2ed7(%rip)        # 404030 <a>
 */