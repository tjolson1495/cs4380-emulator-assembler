#ifndef EMU_4380_H
#define EMU_4380_H

#include <string>

extern unsigned int MEM_SIZE;
extern unsigned int reg_file[22];
extern unsigned char* prog_mem;
extern unsigned int cntrl_regs[5];
extern std::string RegnameStrings[];

enum RegNames{R0=0, R1, R2, R3, R4, R5, R6, R7, R8, R9, R10, R11, R12, R13, R14, R15, PC, SL, SB, SP, FP, HP};

enum CntrlRegNames{OPERATION=0, OPERAND_1, OPERAND_2, OPERAND_3, IMMEDIATE};

enum Operations{JMP=1, MOV=7, MOVI, LDA, STR, LDR, STB, LDB, ADD = 18, ADDI, SUB, SUBI, MUL, MULI, DIV, SDIV, DIVI, TRP=31};

bool fetch();
bool decode();
bool execute();
bool init_mem(unsigned int size);


//helper functions
bool jmp();
bool mov();
bool movi();
bool lda();
bool str();
bool ldr();
bool stb();
bool ldb();
bool add();
bool addi();
bool sub();
bool subi();
bool mul();
bool muli();
bool div();
bool sdiv();
bool divi();

bool trp0();
bool trp1();
bool trp2();
bool trp3();
bool trp4();
bool trp98();

void invalid_instruction(unsigned int address);

// Project Part 3
bool istr();
bool ildr();
bool istb();
bool ildb();
bool jmr();
bool bnz();
bool bgt();
bool blt();
bool brz();
bool cmp();
bool cmpi();

#endif
