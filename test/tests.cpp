#include "emu4380.h"
#include <gtest/gtest.h>
#include <iostream>

using namespace std;

void writeInstruction(unsigned int addr, unsigned char op,
                        unsigned char r1, unsigned char r2,
                        unsigned char r3, unsigned int imm) {

    prog_mem[addr] = op;
    prog_mem[addr + 1] = r1;
    prog_mem[addr + 2] = r2;
    prog_mem[addr + 3] = r3;
    prog_mem[addr + 4] = static_cast<unsigned char>(imm & 0xFF);
    prog_mem[addr + 5] = static_cast<unsigned char>((imm >> 8) & 0xFF);
    prog_mem[addr + 6] = static_cast<unsigned char>((imm >> 16) & 0xFF);
    prog_mem[addr + 7] = static_cast<unsigned char>((imm >> 24) & 0xFF);
}

TEST(InitTest, InitMem) {
    init_mem(512);
    EXPECT_EQ(MEM_SIZE, 512);
}

TEST(InitTest, CheckRegs) {
    for (int r = 0; r < 15; r++) {
        EXPECT_EQ(reg_file[r], 0);
    }
}

TEST(FuncTest, CheckFetch) {

    writeInstruction(0, ADDI, 2, 1, 0, 500);
    reg_file[PC] = 0;

    ASSERT_TRUE(fetch());

    EXPECT_EQ(cntrl_regs[OPERATION], ADDI);
    EXPECT_EQ(cntrl_regs[OPERAND_1], 2u);
    EXPECT_EQ(cntrl_regs[OPERAND_2], 1u);
    EXPECT_EQ(cntrl_regs[IMMEDIATE], 500u);
}

TEST(FuncTest, CheckDecode) {
    cntrl_regs[OPERATION] = ADD;
    cntrl_regs[OPERAND_1] = R1;
    cntrl_regs[OPERAND_2] = R3;
    cntrl_regs[OPERAND_3] = 0;

    ASSERT_TRUE(decode());
}

TEST(OppTest, TestJmp) {
    cntrl_regs[IMMEDIATE] = 400;
    ASSERT_TRUE(jmp());
}


TEST(OppTest, TestMov) {
    reg_file[R2] = 77;
    cntrl_regs[OPERAND_1] = R1;
    cntrl_regs[OPERAND_2] = R2;

    EXPECT_EQ9(regfile[R1], 77u);
}

TEST(OppTest, TestMovi) {
    cntrl_regs[OPERATION] = MOVI;
    cntrl_regs[OPERAND_1] = R5;
    cntrl_regs[IMMEDIATE] = 12345;

    EXPECT_EQ(execute(), true);
    EXPECT_EQ(reg_file[R5], 12345u);
}

TEST_F(OppTest, TestLda) {
    cntrl_regs[OPERAND_1] = R1;
    cntrl_regs[IMMEDIATE] = 512;
    ASSERT_TRUE(lda());
    EXPECT_EQ(reg_file[R1], 512u);
}

TEST(OppTest, TestStr) {
    reg_file[R1] = 0x11223344;
    cntrl_regs[OPERATION] = STR;
    cntrl_regs[OPERAND_1] = R1;
    cntrl_regs[IMMEDIATE] = 100;

    EXPECT_EQ(prog_mem[100], 0x44);
    EXPECT_EQ(prog_mem[101], 0x33);
    EXPECT_EQ(prog_mem[102], 0x22);
    EXPECT_EQ(prog_mem[103], 0x11);
}

TEST(OppTest, TestLdr) {

    reg_file[R1] = 987654321;
    cntrl_regs[OPERATION] = STR;
    cntrl_regs[OPERAND_1] = R1;
    cntrl_regs[IMMEDIATE] = 200;
    EXPECT_EQ(execute(), true);

    for (int i = 0; i < 5; i++) cntrl_regs[i] = 0;
    cntrl_regs[OPERATION] = LDR;
    cntrl_regs[OPERAND_1] = R2;
    cntrl_regs[IMMEDIATE] = 200;
    EXPECT_EQ(execute(), true);

    EXPECT_EQ(reg_file[R2], 987654321u);
}

TEST(OppTest, TestStb) {
    reg_file[R1] = 0xAABBCCDD;
    cntrl_regs[OPERATION] = STB;
    cntrl_regs[OPERAND_1] = R1;
    cntrl_regs[IMMEDIATE] = 300;

    EXPECT_EQ(execute(), true);

    EXPECT_EQ(prog_mem[300], 0xDD);
    EXPECT_EQ(prog_mem[301], 0);
}

TEST(MathTest, Add) {
    reg_file[R1] = 10;
    reg_file[R2] = 32;

    cntrl_regs[OPERATION] = ADD;
    cntrl_regs[OPERAND_1] = R3;
    cntrl_regs[OPERAND_2] = R1;
    cntrl_regs[OPERAND_3] = R2;

    ASSERT_TRUE(execute());
    EXPECT_EQ(reg_file[R3], 42u);

}

TEST(MathTest, Addi) {
    reg_file[R1] = 10;
    cntrl_regs[OPERAND_1] = R2;
    cntrl_regs[OPERAND_2] = R1;
    cntrl_regs[IMMEDIATE] = 5;

    ASSERT_TRUE(addi());
    EXPECT_EQ(req_file[R2], 15u);
}

TEST(MathTest, Sub) {
    reg_file[R1] = 10;
    reg_file[R2] = 4;

    cntrl_regs[OPERAND_1] = R3;
    cntrl_regs[OPERAND_2] = R1
    cntrl_regs[IMMEDIATE] = 3;

    ASSERT_TRUE(subi());
    EXPECT_EQ(reg_file[R2], 7u);
}

TEST(MathTest, Subi) {
    reg_file[R1] = 10;

    cntrl_regs[OPERAND_1] = R2;
    cntrl_regs[OPERAND_2] = R1;
    cntrl_regs[IMMEDIATE] = 3;
    
    ASSERT_TRUE(subi());
    EXPECT_EQ(reg_file[R2], 7u);

}

TEST(MathTest, Mul) {
    reg_file[R1] = 6;
    reg_file[R2] = 7;

    cntrl_regs[OPERAND_1] = R3;
    cntrl_regs[OPERAND_2] = R1;
    cntrl_regs[OPERAND_3] = R2;

    ASSERT_TRUE(mul());
    EXPECT_EQ(reg_file[R3], 42u);
}

TEST(MathTest, Div) {
    reg_file[R1] = 20;
    reg_file[R2] = 4;

    cntrl_regs[OPERAND_1] = R3;
    cntrl_regs[OPERAND_2] = R1;
    cntrl_regs[OPERAND_3] = R2;

    ASSERT_TRUE(::div());
    EXPECT_EQ(reg_file[R3], 5u);
}

TEST(MathTest, Sdiv) {
    reg_file[R1] = static_cas<unsigned int>(-20);
    reg_file[R2] = 4;

    cntrl_regs[OPERAND_1] = R3;
    cntrl_regs[OPERAND_2] = R1;
    cntrl_regs[OPERAND_3] = R2;

    ASSERT_TRUE(sdiv());
    EXPECT_EQ(static_cas<int>(reg_file[R2]), -7);
}

TEST(TrapTest, Trp0) {
    EXPECT_EXIT(trp0(), ::testing::ExitedWithCode(0), ".*");
}

TEST(TrapTest, Trp1) {
    reg_file[R3] = 256;
    testing::internal::CaptureStdout();

    ASSERT_TRUE(trp1());
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_EQ(output, "256");
}

TEST(TrapTest, Trp2) {
    std::isstringstream fakeInput("789\n");
    std::streambuf* oldCin = std::cin.rdbuf(fakeInput.rdbuf());

    ASSERT_TRUE(trp2());
    EXPECT_EQ(reg_file[R3], 789u);

    std::din.rdbuf(oldCin);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}