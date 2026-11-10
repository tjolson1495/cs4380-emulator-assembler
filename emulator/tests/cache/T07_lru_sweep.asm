; T07 - three passes over 65 locations spaced 16 bytes apart starting at 16640 (reads only).
; 65 blocks is one more than a 64-line cache holds, so strict LRU in a fully associative
; cache evicts exactly the block needed next on every access (worst case for LRU),
; while the direct mapped cache only conflicts on the first and last block.
; Prints the number of reads performed: 3*65 = 195.
    movi r9, #3
    movi r3, #0
pass movi r1, #16640
    movi r2, #65
sweep ildr r4, r1
    addi r3, r3, #1
    addi r1, r1, #16
    subi r2, r2, #1
    bnz  r2, sweep
    subi r9, r9, #1
    bnz  r9, pass
    trp  #1
    trp  #0
