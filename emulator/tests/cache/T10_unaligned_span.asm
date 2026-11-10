; T10 - a 4-byte word stored at address 16670 (30 bytes into a 32-byte region), so it
; straddles two blocks in both the 16-byte and 32-byte block configurations.
; It is then read 10 times. Prints 10 * 12345 = 123450.
    movi r1, #16670
    movi r6, #12345
    istr r6, r1
    movi r8, #10
    movi r3, #0
loop ildr r4, r1
    add  r3, r3, r4
    subi r8, r8, #1
    bnz  r8, loop
    trp  #1
    trp  #0
