; T08 - 20 rounds of storing a counter to two addresses 1024 bytes apart (16640, 17664).
; In a direct mapped cache every store evicts the other dirty block (write-back cost);
; a fully associative cache just keeps both lines dirty. The final reads prove the
; evicted data really reached memory: both hold 20, so it prints 40.
    movi r1, #16640
    movi r2, #17664
    movi r8, #20
    movi r6, #0
loop addi r6, r6, #1
    istr r6, r1
    istr r6, r2
    subi r8, r8, #1
    bnz  r8, loop
    ildr r4, r1
    ildr r5, r2
    add  r3, r4, r5
    trp  #1
    trp  #0
