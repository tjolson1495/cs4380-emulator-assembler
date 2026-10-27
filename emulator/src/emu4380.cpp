#include <iostream>

#include "emu4380.h"

unsigned int MEM_SIZE;
unsigned int reg_file[22];
unsigned char* prog_mem;
unsigned int cntrl_regs[5];
std::string RegnameStrings[22] = {
    "R0","R1","R2","R3","R4","R5","R6","R7","R8","R9",
    "R10","R11","R12","R13","R14","R15",
    "PC","SL","SB","SP","FP","HP"
};

bool fetch() {

    unsigned int addr = reg_file[PC];
    if (addr + 8 > MEM_SIZE) return false;

    cntrl_regs[OPERATION] = prog_mem[addr];
    cntrl_regs[OPERAND_1] = prog_mem[addr + 1];
    cntrl_regs[OPERAND_2] = prog_mem[addr + 2];
    cntrl_regs[OPERAND_3] = prog_mem[addr + 3];
    cntrl_regs[IMMEDIATE] = *reinterpret_cast<unsigned int*>(prog_mem + addr + 4);

    reg_file[PC] += 8;

    return true;
}

bool decode() {
    unsigned int opp = cntrl_regs[OPERATION];
    unsigned int op1 = cntrl_regs[OPERAND_1];
    unsigned int op2 = cntrl_regs[OPERAND_2];
    unsigned int op3 = cntrl_regs[OPERAND_3];

    switch(opp) {

        case 1:
            // No aditional verifications
            break;

        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
            // Operand 1 only
            
            if (op1 > 21) return false;

            break;

        case 7:
        case 14:
        case 15:
        case 16:
        case 17:
        case 19:
        case 21:
        case 23:
        case 26:
        case 30:
            // Operand 1 and 2

            if (op1 > 21) return false;
            if (op2 > 21) return false;
            break;

        case 18:
        case 20:
        case 22:
        case 24:
        case 25:
        case 29:
        
            // Operand 1, 2, and 3
            if (op1 > 21) return false;
            if (op2 > 21) return false;
            if (op3 > 21) return false;
            break;

        case 31: {

            unsigned int imm = cntrl_regs[IMMEDIATE];
            if (imm > 4u && imm != 98u) return false;
            break;
        }

        default:

            return false;
    }

    return true;
}

bool execute() {
    unsigned int opp = cntrl_regs[OPERATION];
    bool result = true;

    switch(opp) {
        case 1:  result = jmp();  break;
        case 2:  result = jmr();  break;
        case 3:  result = bnz();  break;
        case 4:  result = bgt();  break;
        case 5:  result = blt();  break;
        case 6:  result = brz();  break;
        case 7:  result = mov();  break;
        case 8:  result = movi(); break;
        case 9:  result = lda();  break;
        case 10: result = str();  break;
        case 11: result = ldr();  break;
        case 12: result = stb();  break;
        case 13: result = ldb();  break;
        case 14: result = istr(); break;
        case 15: result = ildr(); break;
        case 16: result = istb(); break;
        case 17: result = ildb(); break;
        case 18: result = add();  break;
        case 19: result = addi(); break;
        case 20: result = sub();  break;
        case 21: result = subi(); break;
        case 22: result = mul();  break;
        case 23: result = muli(); break;
        case 24: result = div();  break;
        case 25: result = sdiv(); break;
        case 26: result = divi(); break;
        case 29: result = cmp();  break;
        case 30: result = cmpi(); break;
        case 31: {
            unsigned int imm = cntrl_regs[IMMEDIATE];
            if (imm == 0u) result = trp0();
            else if (imm == 1u) result = trp1();
            else if (imm == 2u) result = trp2();
            else if (imm == 3u) result = trp3();
            else if (imm == 4u) result = trp4();
            else if (imm == 98u) result = trp98();
            break;
        }
        default:
            return false;
    }

    return result;
}

bool init_mem(unsigned int size) {
    MEM_SIZE = size;
    prog_mem = new unsigned char[size]();
    return true;
}

bool jmp() { // 1
    reg_file[PC] = cntrl_regs[IMMEDIATE];

    return true;
}

bool jmr() { // 2
    unsigned int RS = cntrl_regs[OPERAND_1];
    reg_file[PC] = reg_file[RS];

    return true;
}

bool bnz() { // 3
    unsigned int RS = cntrl_regs[OPERAND_1];
    if (static_cast<int>(reg_file[RS]) != 0)
        reg_file[PC] = cntrl_regs[IMMEDIATE];

    return true;
}

bool bgt() { // 4
    unsigned int RS = cntrl_regs[OPERAND_1];
    if (static_cast<int>(reg_file[RS]) > 0) 
        reg_file[PC] = cntrl_regs[IMMEDIATE];

    return true;
}

bool blt() { // 5
    unsigned int RS = cntrl_regs[OPERAND_1];
    if (static_cast<int>(reg_file[RS]) < 0) 
        reg_file[PC] = cntrl_regs[IMMEDIATE];

    return true;
}

bool brz() { // 6
    unsigned int RS = cntrl_regs[OPERAND_1];
    if (static_cast<int>(reg_file[RS]) == 0) 
        reg_file[PC] = cntrl_regs[IMMEDIATE];

    return true;
}

bool mov() { // 7
    unsigned int RD = cntrl_regs[OPERAND_1];
    unsigned int RS = cntrl_regs[OPERAND_2];

    reg_file[RD] = reg_file[RS];
    return true;
}

bool movi() { // 8
    unsigned int RD = cntrl_regs[OPERAND_1];

    reg_file[RD] = cntrl_regs[IMMEDIATE];
    return true;
}

bool lda() { // 9
    unsigned int RD = cntrl_regs[OPERAND_1];
    unsigned int addr = cntrl_regs[IMMEDIATE];

    reg_file[RD] = addr;
    return true;
}

bool str() { // 10
    unsigned int RS = cntrl_regs[OPERAND_1];
    unsigned int addr = cntrl_regs[IMMEDIATE];

    *reinterpret_cast<unsigned int*>(prog_mem + addr) = reg_file[RS];
    return true;
}

bool ldr() { // 11
    unsigned int RD = cntrl_regs[OPERAND_1];
    unsigned int addr = cntrl_regs[IMMEDIATE];

    reg_file[RD] = *reinterpret_cast<unsigned int*>(prog_mem + addr);
    return true;
}

bool stb() { // 12
    unsigned int RS = cntrl_regs[OPERAND_1];
    unsigned int addr = cntrl_regs[IMMEDIATE];

    prog_mem[addr] = reg_file[RS] & 0xFF;
    return true;
}

bool ldb() { // 13
    unsigned int RD = cntrl_regs[OPERAND_1];
    unsigned int addr = cntrl_regs[IMMEDIATE];

    reg_file[RD] = prog_mem[addr];

    return true;
}

bool istr() { // 14

    unsigned int RS = cntrl_regs[OPERAND_1];
    unsigned int RG = cntrl_regs[OPERAND_2];

    unsigned int addr = reg_file[RG];

    if (addr > MEM_SIZE - 4 || MEM_SIZE < 4) {
        return false;
    }

    *reinterpret_cast<unsigned int*>(prog_mem + addr) = reg_file[RS];

    return true;
}

bool ildr() { // 15

    unsigned int RS = cntrl_regs[OPERAND_1];
    unsigned int RG = cntrl_regs[OPERAND_2];

    unsigned int addr = reg_file[RG];

    if (addr > MEM_SIZE - 4 || MEM_SIZE < 4) {
        return false;
    }

    reg_file[RS] = *reinterpret_cast<unsigned int*>(prog_mem + addr);

    return true;
}

bool istb() { // 16

    unsigned int RS = cntrl_regs[OPERAND_1];
    unsigned int RG = cntrl_regs[OPERAND_2];

    unsigned int addr = reg_file[RG];

    if (addr >= MEM_SIZE) {
        return false;
    }

    prog_mem[addr] = reg_file[RS];

    return true;
}

bool ildb() { // 17

    unsigned int RS = cntrl_regs[OPERAND_1];
    unsigned int RG = cntrl_regs[OPERAND_2];

    unsigned int addr = reg_file[RG];

    if (addr >= MEM_SIZE) {
        return false;
    }

    reg_file[RS] = prog_mem[addr];

    return true;
}

bool add() { // 18
    unsigned int RD  = cntrl_regs[OPERAND_1];
    unsigned int RS1 = cntrl_regs[OPERAND_2];
    unsigned int RS2 = cntrl_regs[OPERAND_3];

    reg_file[RD] = reg_file[RS1] + reg_file[RS2];
    return true;
}

bool addi() { // 19
    unsigned int RD  = cntrl_regs[OPERAND_1];
    unsigned int RS1 = cntrl_regs[OPERAND_2];
    unsigned int imm = cntrl_regs[IMMEDIATE];

    reg_file[RD] = reg_file[RS1] + imm;
    return true;
}

bool sub() { // 20
    unsigned int RD  = cntrl_regs[OPERAND_1];
    unsigned int RS1 = cntrl_regs[OPERAND_2];
    unsigned int RS2 = cntrl_regs[OPERAND_3];

    reg_file[RD] = reg_file[RS1] - reg_file[RS2];
    return true;
}

bool subi() { // 21
    unsigned int RD  = cntrl_regs[OPERAND_1];
    unsigned int RS1 = cntrl_regs[OPERAND_2];
    unsigned int imm = cntrl_regs[IMMEDIATE];

    reg_file[RD] = reg_file[RS1] - imm;
    return true;
}

bool mul() { // 22
    unsigned int RD  = cntrl_regs[OPERAND_1];
    unsigned int RS1 = cntrl_regs[OPERAND_2];
    unsigned int RS2 = cntrl_regs[OPERAND_3];

    reg_file[RD] = reg_file[RS1] * reg_file[RS2];
    return true;
}

bool muli() { // 23
    unsigned int RD  = cntrl_regs[OPERAND_1];
    unsigned int RS1 = cntrl_regs[OPERAND_2];
    unsigned int imm = cntrl_regs[IMMEDIATE];

    reg_file[RD] = reg_file[RS1] * imm;
    return true;
}

bool div() { // 24
    unsigned int RD  = cntrl_regs[OPERAND_1];
    unsigned int RS1 = cntrl_regs[OPERAND_2];
    unsigned int RS2 = cntrl_regs[OPERAND_3];

    if (reg_file[RS2] == 0) return false;

    reg_file[RD] = reg_file[RS1] / reg_file[RS2];
    return true;
}

bool sdiv() { // 25
    unsigned int RD  = cntrl_regs[OPERAND_1];
    unsigned int RS1 = cntrl_regs[OPERAND_2];
    unsigned int RS2 = cntrl_regs[OPERAND_3];

    if (reg_file[RS2] == 0) return false;

    reg_file[RD] = static_cast<unsigned int>(
        static_cast<int>(reg_file[RS1]) 
        / static_cast<int>(reg_file[RS2])
    );

    return true;
}

bool divi() { // 26
    unsigned int RD  = cntrl_regs[OPERAND_1];
    unsigned int RS1 = cntrl_regs[OPERAND_2];
    unsigned int imm = cntrl_regs[IMMEDIATE];

    if (imm == 0) return false;

    reg_file[RD] = static_cast<unsigned int>(
        static_cast<int>(reg_file[RS1]) 
        / static_cast<int>(imm)
    );

    return true;
}

bool cmp() { // 29
    unsigned int RD = cntrl_regs[OPERAND_1];
    unsigned int RS1 = cntrl_regs[OPERAND_2];
    unsigned int RS2 = cntrl_regs[OPERAND_3];

    if (static_cast<int>(reg_file[RS1]) == static_cast<int>(reg_file[RS2])) {
        reg_file[RD] = 0;
    } else if (static_cast<int>(reg_file[RS1]) > static_cast<int>(reg_file[RS2])) {
        reg_file[RD] = 1;
    } else if (static_cast<int>(reg_file[RS1]) < static_cast<int>(reg_file[RS2])) {
        reg_file[RD] = -1;
    }

    return true;
}

bool cmpi() { // 30
    unsigned int RD = cntrl_regs[OPERAND_1];
    unsigned int RS1 = cntrl_regs[OPERAND_2];
    unsigned int imm = cntrl_regs[IMMEDIATE];

    if (static_cast<int>(reg_file[RS1]) == static_cast<int>(imm)) {
        reg_file[RD] = 0;
    } else if (static_cast<int>(reg_file[RS1]) > static_cast<int>(imm)) {
        reg_file[RD] = 1;
    } else if (static_cast<int>(reg_file[RS1]) < static_cast<int>(imm)) {
        reg_file[RD] = -1;
    }

    return true;
}

bool trp0() { // 31
    std::exit(0);
    return true;
}

bool trp1() { // 31
    std::cout << reg_file[R3];
    return true;
}

bool trp2() { // 31
    std::cin >> reg_file[R3];
    return true;
}

bool trp3() { // 31
    std::cout << static_cast<char>(reg_file[R3]);
    return true;
}

bool trp4() { // 31
    unsigned char input;
    std::cin >> input;
    reg_file[R3] = input;
    return true;
}

bool trp98() { // 31

    for (long unsigned int i = 0; i < std::size(reg_file); i++) {
        std::cout << RegnameStrings[i] << "\t" << reg_file[i] << std::endl;
    }

    return true;
}

void invalid_instruction(unsigned int address) {
    std::cout << "INVALID INSTRUCTION AT: " << address << std::endl;
    std::exit(1);
}