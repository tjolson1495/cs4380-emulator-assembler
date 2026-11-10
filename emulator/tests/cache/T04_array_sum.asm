; T04 - sum of a 16-integer array (1..16) stored in the data segment.
; Sequential reads: spatial locality. Prints 136.
arr .INT #1
    .INT #2
    .INT #3
    .INT #4
    .INT #5
    .INT #6
    .INT #7
    .INT #8
    .INT #9
    .INT #10
    .INT #11
    .INT #12
    .INT #13
    .INT #14
    .INT #15
    .INT #16
    lda  r1, arr
    movi r2, #16
    movi r3, #0
loop ildr r4, r1
    add  r3, r3, r4
    addi r1, r1, #4
    subi r2, r2, #1
    bnz  r2, loop
    trp  #1
    trp  #0
