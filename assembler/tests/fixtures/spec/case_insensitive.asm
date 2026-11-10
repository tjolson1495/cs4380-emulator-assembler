NUM_A       .int    #40
NUM_B       .Int    #2
H_CHAR      .byt    'H'
I_CHAR      .bYt    'i'
NEWLINE     .BYT    '\n'
MAIN        ldb     r3, H_CHAR
            trp     #3
            Ldb     R3, I_CHAR
            tRp     #3
            lDB     r3, NEWLINE
            TRP     #3
            ldr     r1, NUM_A
            LdR     R2, NUM_B
            add     r3, r1, R2
            Trp     #1
            jmp     DONE
            trP     #98
DONE        trp     #0