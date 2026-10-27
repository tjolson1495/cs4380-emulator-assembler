#include "gtest/gtest.h"
#include "emu4380.h"

class M2Test : public ::testing::Test {
protected:
    void SetUp() override {
        ASSERT_TRUE(init_mem(256));
        for (int i = 0; i < 22; ++i) reg_file[i] = 0;
        for (int i = 0; i < 5; ++i) cntrl_regs[i] = 0;
    }
};

TEST_F(M2Test, Istr) {
    reg_file[R1] = 55;   // value to store
    reg_file[R2] = 40;   // address
    cntrl_regs[OPERAND_1] = R1;
    cntrl_regs[OPERAND_2] = R2;
    ASSERT_TRUE(istr());
    EXPECT_EQ(prog_mem[40], 55);
}

TEST_F(M2Test, Ildr) {
    prog_mem[40] = 7;
    reg_file[R2] = 40;   // address
    cntrl_regs[OPERAND_1] = R1;
    cntrl_regs[OPERAND_2] = R2;
    ASSERT_TRUE(ildr());
    EXPECT_EQ(reg_file[R1], 7u);
}

TEST_F(M2Test, Jmr) {
    reg_file[R1] = 100;
    cntrl_regs[OPERAND_1] = R1;
    ASSERT_TRUE(jmr());
    EXPECT_EQ(reg_file[PC], 100u);
}

TEST_F(M2Test, BnzTaken) {
    reg_file[R1] = 5;    // nonzero -> branch taken
    reg_file[PC] = 0;
    cntrl_regs[OPERAND_1] = R1;
    cntrl_regs[IMMEDIATE] = 48;
    ASSERT_TRUE(bnz());
    EXPECT_EQ(reg_file[PC], 48u);
}

TEST_F(M2Test, CmpLt) {
    reg_file[R1] = static_cast<unsigned int>(-3); // signed -3
    reg_file[R2] = 2;
    cntrl_regs[OPERAND_1] = R3;
    cntrl_regs[OPERAND_2] = R1;
    cntrl_regs[OPERAND_3] = R2;
    ASSERT_TRUE(cmp());
    EXPECT_EQ(static_cast<int>(reg_file[R3]), -1);
}
