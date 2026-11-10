; T05 - write then read back 32 integers spaced 16 bytes apart starting at address 16640.
; With 16-byte blocks every access lands in a new block (no spatial locality);
; with 32-byte blocks two accesses share each block. Prints 0+1+...+31 = 496.
    movi r1, #16640
    movi r2, #32
    movi r4, #0
wr  istr r4, r1
    addi r1, r1, #16
    addi r4, r4, #1
    subi r2, r2, #1
    bnz  r2, wr
    movi r1, #16640
    movi r2, #32
    movi r3, #0
rd  ildr r5, r1
    add  r3, r3, r5
    addi r1, r1, #16
    subi r2, r2, #1
    bnz  r2, rd
    trp  #1
    trp  #0
