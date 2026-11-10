; T02 - straight-line arithmetic, no data memory accesses at all.
; Only instruction fetches touch memory. R3 = ((7+5)+30)*2-4 = 80, printed with TRP 1.
    movi r1, #7
    movi r2, #5
    add  r3, r1, r2
    addi r3, r3, #30
    muli r3, r3, #2
    subi r3, r3, #4
    trp  #1
    trp  #0
