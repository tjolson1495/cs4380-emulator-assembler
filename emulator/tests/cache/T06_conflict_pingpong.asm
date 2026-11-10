; T06 - alternate reads of two addresses exactly 1024 bytes apart (16640 and 17664), 50 times.
; Both addresses map to the same line of a direct mapped cache (in both the 64x16 and
; 32x32 configurations), so they evict each other every time. A fully associative
; cache holds both. Prints 50*(3+4) = 350.
    movi r1, #16640
    movi r2, #17664
    movi r6, #3
    movi r7, #4
    istr r6, r1
    istr r7, r2
    movi r8, #50
    movi r3, #0
loop ildr r4, r1
    ildr r5, r2
    add  r3, r3, r4
    add  r3, r3, r5
    subi r8, r8, #1
    bnz  r8, loop
    trp  #1
    trp  #0
