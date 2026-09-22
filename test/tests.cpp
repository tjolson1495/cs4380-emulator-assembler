#include "emu4380.h"
#include <algorithm>
#include <cstdio>
#include <fstream>
#include "gtest/gtest.h"
#include <iostream>
#include <sstream>
#include <sys/wait.h>

using namespace std;

void writeInstruction(unsigned int addr, unsigned char op, unsigned char r1,
                      unsigned char r2, unsigned char r3, unsigned int imm) {

  prog_mem[addr] = op;
  prog_mem[addr + 1] = r1;
  prog_mem[addr + 2] = r2;
  prog_mem[addr + 3] = r3;
  prog_mem[addr + 4] = static_cast<unsigned char>(imm & 0xFF);
  prog_mem[addr + 5] = static_cast<unsigned char>((imm >> 8) & 0xFF);
  prog_mem[addr + 6] = static_cast<unsigned char>((imm >> 16) & 0xFF);
  prog_mem[addr + 7] = static_cast<unsigned char>((imm >> 24) & 0xFF);
}

class Emu4380Test : public ::testing::Test {
protected:
  void SetUp() override {
    init_mem(512);
    std::fill(std::begin(reg_file), std::end(reg_file), 0u);
    std::fill(std::begin(cntrl_regs), std::end(cntrl_regs), 0u);
  }
};

TEST_F(Emu4380Test, InitMem) {
  init_mem(512);
  EXPECT_EQ(MEM_SIZE, 512u);
}

TEST_F(Emu4380Test, CheckRegs) {
  for (unsigned int r = 0; r < std::size(reg_file); r++) {
    EXPECT_EQ(reg_file[r], 0u);
  }
}

TEST_F(Emu4380Test, CheckFetch) {
  writeInstruction(0, ADDI, 2, 1, 0, 500);
  reg_file[PC] = 0;

  ASSERT_TRUE(fetch());

  EXPECT_EQ(cntrl_regs[OPERATION], ADDI);
  EXPECT_EQ(cntrl_regs[OPERAND_1], 2u);
  EXPECT_EQ(cntrl_regs[OPERAND_2], 1u);
  EXPECT_EQ(cntrl_regs[IMMEDIATE], 500u);
}

TEST_F(Emu4380Test, CheckDecode) {
  cntrl_regs[OPERATION] = ADD;
  cntrl_regs[OPERAND_1] = R1;
  cntrl_regs[OPERAND_2] = R3;
  cntrl_regs[OPERAND_3] = 0;

  ASSERT_TRUE(decode());
}

TEST_F(Emu4380Test, TestJmp) {
  cntrl_regs[IMMEDIATE] = 400;
  ASSERT_TRUE(jmp());
  EXPECT_EQ(reg_file[PC], 400u);
}

TEST_F(Emu4380Test, TestMov) {
  reg_file[R2] = 77;
  cntrl_regs[OPERAND_1] = R1;
  cntrl_regs[OPERAND_2] = R2;

  ASSERT_TRUE(mov());
  EXPECT_EQ(reg_file[R1], 77u);
}

TEST_F(Emu4380Test, TestMovi) {
  cntrl_regs[OPERATION] = MOVI;
  cntrl_regs[OPERAND_1] = R5;
  cntrl_regs[IMMEDIATE] = 12345;

  EXPECT_EQ(execute(), true);
  EXPECT_EQ(reg_file[R5], 12345u);
}

TEST_F(Emu4380Test, TestLda) {
  cntrl_regs[OPERAND_1] = R1;
  cntrl_regs[IMMEDIATE] = 512;
  ASSERT_TRUE(lda());
  EXPECT_EQ(reg_file[R1], 512u);
}

TEST_F(Emu4380Test, TestStr) {
  reg_file[R1] = 0x11223344;
  cntrl_regs[OPERATION] = STR;
  cntrl_regs[OPERAND_1] = R1;
  cntrl_regs[IMMEDIATE] = 100;

  ASSERT_TRUE(execute());

  EXPECT_EQ(prog_mem[100], 0x44);
  EXPECT_EQ(prog_mem[101], 0x33);
  EXPECT_EQ(prog_mem[102], 0x22);
  EXPECT_EQ(prog_mem[103], 0x11);
}

TEST_F(Emu4380Test, TestLdr) {
  reg_file[R1] = 987654321;
  cntrl_regs[OPERATION] = STR;
  cntrl_regs[OPERAND_1] = R1;
  cntrl_regs[IMMEDIATE] = 200;
  EXPECT_EQ(execute(), true);

  for (int i = 0; i < 5; i++)
    cntrl_regs[i] = 0;
  cntrl_regs[OPERATION] = LDR;
  cntrl_regs[OPERAND_1] = R2;
  cntrl_regs[IMMEDIATE] = 200;
  EXPECT_EQ(execute(), true);

  EXPECT_EQ(reg_file[R2], 987654321u);
}

TEST_F(Emu4380Test, TestStb) {
  reg_file[R1] = 0xAABBCCDD;
  cntrl_regs[OPERATION] = STB;
  cntrl_regs[OPERAND_1] = R1;
  cntrl_regs[IMMEDIATE] = 300;

  EXPECT_EQ(execute(), true);

  EXPECT_EQ(prog_mem[300], 0xDD);
  EXPECT_EQ(prog_mem[301], 0);
}

TEST_F(Emu4380Test, Add) {
  reg_file[R1] = 10;
  reg_file[R2] = 32;

  cntrl_regs[OPERATION] = ADD;
  cntrl_regs[OPERAND_1] = R3;
  cntrl_regs[OPERAND_2] = R1;
  cntrl_regs[OPERAND_3] = R2;

  ASSERT_TRUE(execute());
  EXPECT_EQ(reg_file[R3], 42u);
}

TEST_F(Emu4380Test, Addi) {
  reg_file[R1] = 10;
  cntrl_regs[OPERAND_1] = R2;
  cntrl_regs[OPERAND_2] = R1;
  cntrl_regs[IMMEDIATE] = 5;

  ASSERT_TRUE(addi());
  EXPECT_EQ(reg_file[R2], 15u);
}

TEST_F(Emu4380Test, Sub) {
  reg_file[R1] = 10;
  reg_file[R2] = 3;

  cntrl_regs[OPERAND_1] = R3;
  cntrl_regs[OPERAND_2] = R1;
  cntrl_regs[OPERAND_3] = R2;

  ASSERT_TRUE(sub());
  EXPECT_EQ(reg_file[R3], 7u);
}

TEST_F(Emu4380Test, Subi) {
  reg_file[R1] = 10;

  cntrl_regs[OPERAND_1] = R2;
  cntrl_regs[OPERAND_2] = R1;
  cntrl_regs[IMMEDIATE] = 3;

  ASSERT_TRUE(subi());
  EXPECT_EQ(reg_file[R2], 7u);
}

TEST_F(Emu4380Test, Mul) {
  reg_file[R1] = 6;
  reg_file[R2] = 7;

  cntrl_regs[OPERAND_1] = R3;
  cntrl_regs[OPERAND_2] = R1;
  cntrl_regs[OPERAND_3] = R2;

  ASSERT_TRUE(mul());
  EXPECT_EQ(reg_file[R3], 42u);
}

TEST_F(Emu4380Test, Div) {
  reg_file[R1] = 20;
  reg_file[R2] = 4;

  cntrl_regs[OPERAND_1] = R3;
  cntrl_regs[OPERAND_2] = R1;
  cntrl_regs[OPERAND_3] = R2;

  ASSERT_TRUE(::div());
  EXPECT_EQ(reg_file[R3], 5u);
}

TEST_F(Emu4380Test, Sdiv) {
  reg_file[R1] = static_cast<unsigned int>(-20);
  reg_file[R2] = 4;

  cntrl_regs[OPERAND_1] = R3;
  cntrl_regs[OPERAND_2] = R1;
  cntrl_regs[OPERAND_3] = R2;

  ASSERT_TRUE(sdiv());
  EXPECT_EQ(static_cast<int>(reg_file[R3]), -5);
}

TEST_F(Emu4380Test, Trp0) {
  EXPECT_EXIT(trp0(), ::testing::ExitedWithCode(0), ".*");
}

TEST_F(Emu4380Test, Trp1) {
  reg_file[R3] = 256;
  testing::internal::CaptureStdout();

  ASSERT_TRUE(trp1());
  std::string output = testing::internal::GetCapturedStdout();

  EXPECT_EQ(output, "256");
}

TEST_F(Emu4380Test, Trp2) {
  std::istringstream fakeInput("789\n");
  std::streambuf *oldCin = std::cin.rdbuf(fakeInput.rdbuf());

  ASSERT_TRUE(trp2());
  EXPECT_EQ(reg_file[R3], 789u);

  std::cin.rdbuf(oldCin);
}

TEST_F(Emu4380Test, Trp3) {
  reg_file[R3] = static_cast<unsigned int>('A');
  testing::internal::CaptureStdout();

  ASSERT_TRUE(trp3());
  std::string output = testing::internal::GetCapturedStdout();

  EXPECT_EQ(output, "A");
}

TEST_F(Emu4380Test, Trp4) {
  std::istringstream fakeInput("Q");
  std::streambuf *oldCin = std::cin.rdbuf(fakeInput.rdbuf());

  ASSERT_TRUE(trp4());
  EXPECT_EQ(reg_file[R3], static_cast<unsigned int>('Q'));

  std::cin.rdbuf(oldCin);
}

TEST_F(Emu4380Test, Trp98) {
  reg_file[R1] = 42;
  testing::internal::CaptureStdout();

  ASSERT_TRUE(trp98());
  std::string output = testing::internal::GetCapturedStdout();

  EXPECT_NE(output.find("R1\t42"), std::string::npos);
}

void createBinaryFile(const string &filename, unsigned char bytes[],
                      size_t size) {
  ofstream out(filename, ios::binary);
  out.write(reinterpret_cast<const char *>(bytes), size);
  out.close();
}

TEST(SystemTests, TRP98Test) {
  const unsigned char address = 4;
  const unsigned char trp = 31;
  string filename = "trp98_test.bin";
  unsigned char bytes[] = {address, 0, 0,   0, trp, 0, 0, 0, 98, 0,
                           0,       0, trp, 0, 0,   0, 0, 0, 0,  0};
  createBinaryFile(filename, bytes, sizeof(bytes));
  FILE *file = fopen(filename.c_str(), "rb");
  ASSERT_NE(file, nullptr);
  fclose(file);
  FILE *pipe = popen(("./emu4380 " + filename).c_str(), "r");
  ASSERT_NE(pipe, nullptr);
  string output;
  char buffer[256];
  while (fgets(buffer, sizeof(buffer), pipe)) {
    output += buffer;
  }
  int status = pclose(pipe);
  ASSERT_TRUE(WIFEXITED(status));
  ASSERT_EQ(WEXITSTATUS(status), 0);
  EXPECT_NE(output.find("R0\t0\n"), string::npos);
  EXPECT_NE(output.find("R1\t0\n"), string::npos);
  EXPECT_NE(output.find("R2\t0\n"), string::npos);
  EXPECT_NE(output.find("R3\t0\n"), string::npos);
  EXPECT_NE(output.find("R4\t0\n"), string::npos);
  EXPECT_NE(output.find("R5\t0\n"), string::npos);
  EXPECT_NE(output.find("R6\t0\n"), string::npos);
  EXPECT_NE(output.find("R7\t0\n"), string::npos);
  EXPECT_NE(output.find("R8\t0\n"), string::npos);
  EXPECT_NE(output.find("R9\t0\n"), string::npos);
  EXPECT_NE(output.find("R10\t0\n"), string::npos);
  EXPECT_NE(output.find("R11\t0\n"), string::npos);
  EXPECT_NE(output.find("R12\t0\n"), string::npos);
  EXPECT_NE(output.find("R13\t0\n"), string::npos);
  EXPECT_NE(output.find("R14\t0\n"), string::npos);

  EXPECT_NE(output.find("R15\t0\n"), string::npos);
  EXPECT_NE(output.find("PC\t12\n"), string::npos);
  EXPECT_NE(output.find("SL\t20\n"), string::npos);
  EXPECT_NE(output.find("SB\t131072\n"), string::npos);
  EXPECT_NE(output.find("SP\t131072\n"), string::npos);
  EXPECT_NE(output.find("FP\t0\n"), string::npos);
  EXPECT_NE(output.find("HP\t20\n"), string::npos);
}

TEST(SystemTests, InsufficientMemoryTest) {
  const unsigned char address = 4;
  const unsigned char addi = 19;
  const unsigned char R1 = 1;
  string filename = "insufficient_memory_test.bin";
  unsigned char bytes[] = {address, 0, 0, 0, addi, R1, R1, 0, 10, 0, 0, 0};
  createBinaryFile(filename, bytes, sizeof(bytes));
  FILE *file = fopen(filename.c_str(), "rb");
  ASSERT_NE(file, nullptr);
  fclose(file);
  FILE *pipe = popen(("./emu4380 " + filename + " 8").c_str(), "r");
  ASSERT_NE(pipe, nullptr);
  string output;
  char buffer[256];
  while (fgets(buffer, sizeof(buffer), pipe)) {
    output += buffer;
  }
  int status = pclose(pipe);
  ASSERT_TRUE(WIFEXITED(status));
  ASSERT_EQ(WEXITSTATUS(status), 2);
  EXPECT_EQ(output, "INSUFFICIENT MEMORY SPACE\n");
}

TEST(SystemTests, InvalidInstructionTest) {
  const unsigned char address = 4;
  const unsigned char invalid = 32;
  string filename = "invalid_instruction_test.bin";
  unsigned char bytes[] = {address, 0, 0, 0, invalid, 0, 0, 0, 0, 0, 0, 0};
  createBinaryFile(filename, bytes, sizeof(bytes));
  FILE *file = fopen(filename.c_str(), "rb");
  ASSERT_NE(file, nullptr);
  fclose(file);
  FILE *pipe = popen(("./emu4380 " + filename).c_str(), "r");

  ASSERT_NE(pipe, nullptr);
  string output;
  char buffer[256];
  while (fgets(buffer, sizeof(buffer), pipe)) {
    output += buffer;
  }
  int status = pclose(pipe);
  ASSERT_TRUE(WIFEXITED(status));
  ASSERT_EQ(WEXITSTATUS(status), 1);
  EXPECT_EQ(output, "INVALID INSTRUCTION AT: 4\n");
}

TEST(SystemTests, TRP1PrintIntTest) {
  const unsigned char address = 4;
  const unsigned char movi = 8;
  const unsigned char trp = 31;
  string filename = "trp1_test.bin";
  unsigned char bytes[] = {address, 0, 0,   0, movi, R3, 0, 0, 42, 0,
                           0,       0, trp, 0, 0,    0,  1, 0, 0,  0,
                           trp,     0, 0,   0, 0,    0,  0, 0};
  createBinaryFile(filename, bytes, sizeof(bytes));
  FILE *file = fopen(filename.c_str(), "rb");
  ASSERT_NE(file, nullptr);
  fclose(file);
  FILE *pipe = popen(("./emu4380 " + filename).c_str(), "r");
  ASSERT_NE(pipe, nullptr);
  string output;
  char buffer[256];
  while (fgets(buffer, sizeof(buffer), pipe)) {
    output += buffer;
  }
  int status = pclose(pipe);
  ASSERT_TRUE(WIFEXITED(status));
  ASSERT_EQ(WEXITSTATUS(status), 0);
  EXPECT_EQ(output, "42");
}

TEST(IntegrationTests, MultiInstructionSequenceTest) {
  reg_file[PC] = 0;
  init_mem(17);
  prog_mem[0] = MOVI;
  prog_mem[1] = R1;
  prog_mem[2] = 0;
  prog_mem[3] = 0;
  prog_mem[4] = 10;
  prog_mem[5] = 0;
  prog_mem[6] = 0;
  prog_mem[7] = 0;

  prog_mem[8] = ADDI;
  prog_mem[9] = R2;
  prog_mem[10] = R1;
  prog_mem[11] = 0;
  prog_mem[12] = 5;
  prog_mem[13] = 0;
  prog_mem[14] = 0;
  prog_mem[15] = 0;
  fetch();
  ASSERT_TRUE(decode());
  execute();
  fetch();
  ASSERT_TRUE(decode());
  execute();
  EXPECT_EQ(reg_file[R1], 10);
  EXPECT_EQ(reg_file[R2], 15);
}

TEST(UnitTests, MemoryInitializedTest) {
  init_mem(8);
  for (int i = 0; i < 8; i++) {
    ASSERT_EQ(0, prog_mem[i]);
  }
}

TEST(UnitTests, FetchFunctionTest) {
  reg_file[PC] = 0;
  init_mem(8);
  prog_mem[0] = TRP;
  prog_mem[1] = 0;
  prog_mem[2] = 0;
  prog_mem[3] = 0;
  prog_mem[4] = 0;
  prog_mem[5] = 0;
  prog_mem[6] = 0;
  prog_mem[7] = 0;
  ASSERT_TRUE(fetch());
  reg_file[PC] = 0;
  init_mem(7);
  prog_mem[0] = TRP;
  prog_mem[1] = 0;
  prog_mem[2] = 0;
  prog_mem[3] = 0;
  prog_mem[4] = 0;
  prog_mem[5] = 0;
  prog_mem[6] = 0;
  prog_mem[7] = 0;
  ASSERT_FALSE(fetch());
}

TEST(UnitTests, DecodeInvalidOpcodeTest) {

  reg_file[PC] = 0;
  init_mem(9);
  prog_mem[0] = 97;
  prog_mem[1] = R5;
  prog_mem[2] = 0;
  prog_mem[3] = 0;
  prog_mem[4] = 42;
  prog_mem[5] = 0;
  prog_mem[6] = 0;
  prog_mem[7] = 0;

  // invalid opcode

  fetch();
  ASSERT_FALSE(decode());
}

TEST(UnitTests, DecodeInvalidOperandTest) {
  reg_file[PC] = 0;
  init_mem(9);
  prog_mem[0] = MOV;
  prog_mem[1] = 55;
  prog_mem[2] = R1;
  prog_mem[3] = 0;
  prog_mem[4] = 0;
  prog_mem[5] = 0;
  prog_mem[6] = 0;
  prog_mem[7] = 0;

  // invalid register

  fetch();
  ASSERT_FALSE(decode());
}

TEST(UnitTests, DivideByZeroTest) {
  reg_file[PC] = 0;
  reg_file[R1] = 10;
  reg_file[R2] = 0;
  init_mem(9);
  prog_mem[0] = DIV;
  prog_mem[1] = R3;
  prog_mem[2] = R1;
  prog_mem[3] = R2;
  prog_mem[4] = 0;
  prog_mem[5] = 0;
  prog_mem[6] = 0;
  prog_mem[7] = 0;
  fetch();
  ASSERT_TRUE(decode());
  ASSERT_FALSE(execute());
}

TEST(memTest, BasicAssertions) {
  EXPECT_TRUE(init_mem(200));
  EXPECT_EQ(MEM_SIZE, 200);
  // trp 98 printout
  cntrl_regs[0] = TRP;
  cntrl_regs[4] = 98;
  execute();
}

TEST(regTest, BasicAssertaions) {
  unsigned char regTestMem[16] = {
      LDA, R1, 0, 0, 0, 0, 0, 5,
      TRP, 0,  0, 0, 0, 0, 0, 0};

  init_mem(16);
  std::fill(std::begin(reg_file), std::end(reg_file), 0u);
  reg_file[PC] = 0;
  for (int i = 0; i < 16; i++) prog_mem[i] = regTestMem[i];

  for (int i = 0; i < 16; i++) {
    EXPECT_EQ(reg_file[i], 0);
  }
  EXPECT_TRUE(fetch());
  EXPECT_TRUE(decode());
  EXPECT_EQ(reg_file[FP], 0);
  EXPECT_TRUE(execute());

  EXPECT_TRUE(fetch());
  EXPECT_TRUE(decode());
  EXPECT_EXIT(execute(), ::testing::ExitedWithCode(0), ".*");  // TRP #0
}

TEST(addTest, BasicAssertions) {
  reg_file[0] = 55;
  reg_file[1] = 32;
  reg_file[2] = 5;
  reg_file[3] = 22;
  cntrl_regs[0] = ADD;
  cntrl_regs[1] = R0;
  cntrl_regs[2] = R1;
  cntrl_regs[3] = R2;
  cntrl_regs[4] = 4;
  execute();
  EXPECT_EQ(reg_file[0], 37);
  EXPECT_EQ(reg_file[1], 32);
  EXPECT_EQ(reg_file[2], 5);
  EXPECT_EQ(reg_file[3], 22);
}

TEST(subTest, BasicAssertions) {
  reg_file[0] = 55;
  reg_file[1] = 32;
  reg_file[2] = 5;
  reg_file[3] = 22;
  cntrl_regs[0] = SUB;
  cntrl_regs[1] = R0;
  cntrl_regs[2] = R1;
  cntrl_regs[3] = R2;
  cntrl_regs[4] = 2;

  execute();
  EXPECT_EQ(reg_file[0], 27);
  EXPECT_EQ(reg_file[1], 32);
  EXPECT_EQ(reg_file[2], 5);
  EXPECT_EQ(reg_file[3], 22);
}

TEST(mulTest, BasicAssertions) {
  reg_file[0] = 55;
  reg_file[1] = 32;
  reg_file[2] = 5;
  reg_file[3] = 22;
  cntrl_regs[0] = MUL;
  cntrl_regs[1] = R0;
  cntrl_regs[2] = R1;
  cntrl_regs[3] = R2;
  cntrl_regs[4] = 1;
  execute();
  EXPECT_EQ(reg_file[0], 160);
  EXPECT_EQ(reg_file[1], 32);
  EXPECT_EQ(reg_file[2], 5);
  EXPECT_EQ(reg_file[3], 22);
}

TEST(divTest, BasicAssertions) {
  reg_file[0] = 55;
  reg_file[1] = 32;
  reg_file[2] = 5;
  reg_file[3] = 22;
  cntrl_regs[0] = DIV;
  cntrl_regs[1] = R0;
  cntrl_regs[2] = R1;
  cntrl_regs[3] = R2;
  cntrl_regs[4] = 1;
  execute();
  EXPECT_EQ(reg_file[0], 6);
  EXPECT_EQ(reg_file[1], 32);
  EXPECT_EQ(reg_file[2], 5);
  EXPECT_EQ(reg_file[3], 22);
}

TEST(addiTest, BasicAssertions) {
  reg_file[0] = 55;
  reg_file[1] = 32;
  reg_file[2] = 5;
  reg_file[3] = 22;
  cntrl_regs[0] = ADDI;
  cntrl_regs[1] = R0;
  cntrl_regs[2] = R1;
  cntrl_regs[3] = R2;
  cntrl_regs[4] = 1;
  execute();
  EXPECT_EQ(reg_file[0], 33);
  EXPECT_EQ(reg_file[1], 32);
  EXPECT_EQ(reg_file[2], 5);
  EXPECT_EQ(reg_file[3], 22);
}

TEST(subiTest, BasicAssertions) {

  reg_file[0] = 55;
  reg_file[1] = 32;
  reg_file[2] = 5;
  reg_file[3] = 22;
  cntrl_regs[0] = SUBI;
  cntrl_regs[1] = R0;
  cntrl_regs[2] = R1;
  cntrl_regs[3] = R2;
  cntrl_regs[4] = 1;
  execute();
  EXPECT_EQ(reg_file[0], 31);
  EXPECT_EQ(reg_file[1], 32);
  EXPECT_EQ(reg_file[2], 5);
  EXPECT_EQ(reg_file[3], 22);
}

TEST(muliTest, BasicAssertions) {
  reg_file[0] = 55;
  reg_file[1] = 32;
  reg_file[2] = 5;
  reg_file[3] = 22;
  cntrl_regs[0] = MULI;
  cntrl_regs[1] = R0;
  cntrl_regs[2] = R1;
  cntrl_regs[3] = R2;
  cntrl_regs[4] = 2;
  execute();
  EXPECT_EQ(reg_file[0], 64);
  EXPECT_EQ(reg_file[1], 32);
  EXPECT_EQ(reg_file[2], 5);
  EXPECT_EQ(reg_file[3], 22);
}

TEST(diviTest, BasicAssertions) {
  reg_file[0] = 56;
  reg_file[1] = 30;
  reg_file[2] = 5;
  reg_file[3] = 22;
  cntrl_regs[0] = DIVI;
  cntrl_regs[1] = R0;
  cntrl_regs[2] = R1;
  cntrl_regs[3] = R2;
  cntrl_regs[4] = 5;
  execute();
  EXPECT_EQ(reg_file[0], 6);
  EXPECT_EQ(reg_file[1], 30);
  EXPECT_EQ(reg_file[2], 5);
  EXPECT_EQ(reg_file[3], 22);
}

TEST(sdivTest, BasicAssertions) {
  reg_file[0] = 55;
  reg_file[1] = 32;
  reg_file[2] = 4;
  reg_file[3] = 22;
  cntrl_regs[0] = SDIV;
  cntrl_regs[1] = R0;
  cntrl_regs[2] = R1;
  cntrl_regs[3] = R2;

  cntrl_regs[4] = 2;
  execute();
  EXPECT_EQ(reg_file[0], 8);
  EXPECT_EQ(reg_file[1], 32);
  EXPECT_EQ(reg_file[2], 4);
  EXPECT_EQ(reg_file[3], 22);
}

TEST(movTest, BasicAssertions) {
  reg_file[0] = 55;
  reg_file[1] = 32;
  reg_file[2] = 4;
  reg_file[3] = 22;
  cntrl_regs[0] = MOV;
  cntrl_regs[1] = R0;
  cntrl_regs[2] = R1;
  cntrl_regs[3] = R2;
  cntrl_regs[4] = 2;
  execute();
  EXPECT_EQ(reg_file[0], 32);
  EXPECT_EQ(reg_file[1], 32);
  EXPECT_EQ(reg_file[2], 4);
  EXPECT_EQ(reg_file[3], 22);
}

TEST(moviTest, BasicAssertions) {
  reg_file[0] = 55;
  reg_file[1] = 32;
  reg_file[2] = 4;
  reg_file[3] = 22;
  cntrl_regs[0] = MOVI;
  cntrl_regs[1] = R0;
  cntrl_regs[2] = R1;
  cntrl_regs[3] = R2;
  cntrl_regs[4] = 2;
  execute();
  EXPECT_EQ(reg_file[0], 2);
  EXPECT_EQ(reg_file[1], 32);
  EXPECT_EQ(reg_file[2], 4);
  EXPECT_EQ(reg_file[3], 22);
}

TEST(ldaTest, BasicAssertions) {
  reg_file[0] = 55;
  reg_file[1] = 32;
  reg_file[2] = 4;
  reg_file[3] = 22;
  cntrl_regs[0] = LDA;
  cntrl_regs[1] = R0;
  cntrl_regs[2] = R1;
  cntrl_regs[3] = R2;
  unsigned int addr = 55;
  cntrl_regs[4] = addr;
  execute();
  EXPECT_EQ(reg_file[0], addr);
  EXPECT_EQ(reg_file[1], 32);
  EXPECT_EQ(reg_file[2], 4);
  EXPECT_EQ(reg_file[3], 22);
}

TEST(trp0Test, BasicAssertaions) {
  unsigned char trpTestMem[24] = {LDA, R1, 0, 0, 5, 0, 0, 0,    TRP,
                                  0,   0,  0, 0, 0, 0, 0, MOVI, R1,
                                  0,   0,  7, 0, 0, 0}; // should hit TRP before
                                                          // setting R1 to 7
  init_mem(24);
  std::fill(std::begin(reg_file), std::end(reg_file), 0u);
  reg_file[PC] = 0;
  for (int i = 0; i < 24; i++) prog_mem[i] = trpTestMem[i];

  EXPECT_TRUE(fetch());
  EXPECT_TRUE(decode());
  EXPECT_TRUE(execute());          // LDA
  EXPECT_EQ(reg_file[R1], 5u);

  EXPECT_TRUE(fetch());
  EXPECT_TRUE(decode());
  EXPECT_EXIT(execute(), ::testing::ExitedWithCode(0), ".*");  // TRP #0
  // Nothing can run after this in-process -- execute() genuinely calls
  // std::exit(0) here. EXPECT_EXIT itself is what proves the program
  // stopped at TRP before the MOVI (which would set R1 to 7) ever ran.
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
