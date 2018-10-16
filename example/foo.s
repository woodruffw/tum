cmp gp0, gp0

mov gp0, 0xAA
mov gp1, 0xBB
mov gp2, 0xFEEDFACE

add gp0, gp1

; induce a fault by trying to jump
; to an unaligned address
mov gp4, 0x1
jmp gp4
