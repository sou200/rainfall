stack_top   = 0x7ffffffff000
rsp_at_main = 0x7fffffffe260

print(hex(stack_top - rsp_at_main))