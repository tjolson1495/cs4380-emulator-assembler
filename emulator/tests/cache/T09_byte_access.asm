; T09 - write bytes 0..63 to 64 consecutive addresses from 16640, then read them back and sum.
; Byte accesses: after one miss the rest of the block is hit. Prints 0+1+...+63 = 2016.
    movi r1, #16640
    movi r2, #64
    movi r4, #0
wr  istb r4, r1
    addi r1, r1, #1
    addi r4, r4, #1
    subi r2, r2, #1
    bnz  r2, wr
    movi r1, #16640
    movi r2, #64
    movi r3, #0
rd  ildb r5, r1
    add  r3, r3, r5
    addi r1, r1, #1
    subi r2, r2, #1
    bnz  r2, rd
    trp  #1
    trp  #0
