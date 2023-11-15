int g1 = 1, g2;
int f;


void func_2(char a, int *b) {
    *b = 2*a;
    g2 = 0;
    if (f)
        *b = a;
}

int main() {
    func_2(0, &g1);
}

/*
 write after read
 
 home/csmith_clang/examples/output2.c:8
  401139:	8b 05 f1 2e 00 00    	mov    0x2ef1(%rip),%eax        # 404030 <f>
/home/csmith_clang/examples/output2.c:7
  40113f:	c7 05 eb 2e 00 00 00 	movl   $0x0,0x2eeb(%rip)        # 404034 <g2>
  401146:	00 00 00 
/home/csmith_clang/examples/output2.c:8
  401149:	85 c0                	test   %eax,%eax
  40114b:	75 02                	jne    40114f <func_2+0x1f>
/home/csmith_clang/examples/output2.c:9
  40114d:	8b 3e                	mov    (%rsi),%edi
  40114f:	89 3e                	mov    %edi,(%rsi)
 */