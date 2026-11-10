; T03 - tight countdown loop, 100 iterations, no data memory accesses.
; Pure temporal locality of instruction fetches. Nothing is printed except the cycle message.
    movi r1, #100
loop subi r1, r1, #1
    bnz  r1, loop
    trp  #0
