NUM_A       .INT    #40
NUM_B       .INT    #2
H_CHAR      .BYT    'H'
I_CHAR      .BYT    'i'
NEWLINE     .BYT    '\n'
MAIN        LDB     R3, H_CHAR
            TRP     #3
            LDB     R3, I_CHAR
            TRP     #3
            LDB     R3, NEWLINE
            TRP     #3
            LDR     R1, NUM_A
            LDR     R2, NUM_B
            ADD     R3, R1, R2
            TRP     #1
            JMP     DONE        ; forward reference
            TRP     #98         ; skipped by the jump
DONE        TRP     #0