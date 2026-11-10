; T12 - data correctness through the cache. Stores 11,22,...,88 at eight addresses 1024 bytes
; apart (all colliding in a direct mapped cache), then reads them back in reverse order.
; Every value must survive eviction and write-back. Prints 11+22+...+88 = 396.
    movi r1, #16640
    movi r4, #0
    movi r8, #8
wr  addi r4, r4, #11
    istr r4, r1
    addi r1, r1, #1024
    subi r8, r8, #1
    bnz  r8, wr
    movi r8, #8
    movi r3, #0
rd  subi r1, r1, #1024
    ildr r5, r1
    add  r3, r3, r5
    subi r8, r8, #1
    bnz  r8, rd
    trp  #1
    trp  #0
