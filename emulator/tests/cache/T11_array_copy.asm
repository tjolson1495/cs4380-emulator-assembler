; T11 - copy a 16-integer array (1..16) from the data segment to address 16640, then sum the copy.
; Sequential reads and writes at two locations. Prints 136.
src .INT #1
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
    lda  r1, src
    movi r2, #16640
    movi r8, #16
copy ildr r4, r1
    istr r4, r2
    addi r1, r1, #4
    addi r2, r2, #4
    subi r8, r8, #1
    bnz  r8, copy
    movi r2, #16640
    movi r8, #16
    movi r3, #0
sum ildr r4, r2
    add  r3, r3, r4
    addi r2, r2, #4
    subi r8, r8, #1
    bnz  r8, sum
    trp  #1
    trp  #0
