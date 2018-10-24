; sum
; sum up to n (n < 4)
; n > 4 is calculated correctly, but not
; printed correctly

; set a halt point somewhere higher up in memory,
; which we can jump when the user gives us n=0
mov gp0, 0x0
sto gp0, 0xFFF0
mov gp5, 0xFFF0

ior gp0
mov gp1, 0x30
mov gp2, 0x0 ; accumulator
mov gp3, 0x0
mov gp4, 0x1

; convert ASCII digit into digit
sub gp0, gp1

cmp gp0, gp3
jeq gp5

; store the ip so that we can jump back here again
sip gp7

add gp2, gp0
sub gp0, gp4
cmp gp0, gp3
jne gp7

; convert back into ASCII
add gp2, gp1
iow gp2
mov gp4, 0xa
iow gp4

hlt
