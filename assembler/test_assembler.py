import unittest
import filecmp
import subprocess
import sys

class TestAssembler(unittest.TestCase):
    def test_jmr(self):
        asmin = "./testasmfiles/JMRTest.asm"
        binout = "./testasmfiles/JMRTest.bin"
        subprocess.run([sys.executable, "asm4380.py", asmin])
        asmcomp = "./testbinfiles/JMRTest.bin"

        self.assertTrue(filecmp.cmp(asmcomp, binout, shallow=False))

    def test_BGT_no_jump(self):
        asmin = "./testasmfiles/BGT_no_jump.asm"
        binout = "./testasmfiles/BGT_no_jump.bin"
        subprocess.run([sys.executable, "asm4380.py", asmin])
        asmcomp = "./testbinfiles/BGTNoJump.bin"

        self.assertTrue(filecmp.cmp(asmcomp, binout, shallow=False))

    def test_BLT_no_jump(self):
        asmin = "./testasmfiles/BLT_no_jump.asm"
        binout = "./testasmfiles/BLT_no_jump.bin"
        subprocess.run([sys.executable, "asm4380.py", asmin])
        asmcomp = "./testbinfiles/BLTNoJump.bin"

        self.assertTrue(filecmp.cmp(asmcomp, binout, shallow=False))

    def test_BNZ_no_jump(self):
        asmin = "./testasmfiles/BNZ_no_jump.asm"
        binout = "./testasmfiles/BNZ_no_jump.bin"
        subprocess.run([sys.executable, "asm4380.py", asmin])
        asmcomp = "./testbinfiles/BNZNoJump.bin"

        self.assertTrue(filecmp.cmp(asmcomp, binout, shallow=False))

    def test_BNZ_with_jump(self):
        asmin = "./testasmfiles/BNZ_with_jump.asm"
        binout = "./testasmfiles/BNZ_with_jump.bin"
        subprocess.run([sys.executable, "asm4380.py", asmin])
        asmcomp = "./testbinfiles/BNZJump.bin"

        self.assertTrue(filecmp.cmp(asmcomp, binout, shallow=False))

    def test_BRZ_no_jump(self):
        asmin = "./testasmfiles/BRZ_no_jump.asm"
        binout = "./testasmfiles/BRZ_no_jump.bin"
        subprocess.run([sys.executable, "asm4380.py", asmin])
        asmcomp = "./testbinfiles/BRZNoJump.bin"

        self.assertTrue(filecmp.cmp(asmcomp, binout, shallow=False))

    def test_BRZ_with_jump(self):
        asmin = "./testasmfiles/BRZ_with_jump.asm"
        binout = "./testasmfiles/BRZ_with_jump.bin"
        subprocess.run([sys.executable, "asm4380.py", asmin])
        asmcomp = "./testbinfiles/BRZJump.bin"

        self.assertTrue(filecmp.cmp(asmcomp, binout, shallow=False))

    def test_ex_asm(self): #example.asm from assembler mark 2 in canvas
        asmin = "./testasmfiles/example.asm"
        binout = "./testasmfiles/example.bin"
        subprocess.run([sys.executable, "asm4380.py", asmin])
        asmcomp = "./testbinfiles/example.bin"

        self.assertTrue(filecmp.cmp(asmcomp, binout, shallow=False))

    def test_minimal(self): #trap 0 with no comments
        asmin = "./testasmfiles/minimal_without_comment.asm"
        binout = "./testasmfiles/minimal_without_comment.bin"
        subprocess.run([sys.executable, "asm4380.py", asmin])
        asmcomp = "./testbinfiles/minimal.bin"

        self.assertTrue(filecmp.cmp(asmcomp, binout, shallow=False))

    def test_minimal_with_comments(self): #trap 0 with comment
        asmin = "./testasmfiles/minimal_with_comment.asm"
        binout = "./testasmfiles/minimal_with_comment.bin"
        subprocess.run([sys.executable, "asm4380.py", asmin])
        asmcomp = "./testbinfiles/minimal.bin"

        self.assertTrue(filecmp.cmp(asmcomp, binout, shallow=False))

    def test_default_int(self): #int with no indicated number and then a trap 0
        asmin = "./testasmfiles/defaultint_without_comment.asm"
        binout = "./testasmfiles/defaultint_without_comment.bin"
        subprocess.run([sys.executable, "asm4380.py", asmin])
        asmcomp = "./testbinfiles/defaultint.bin"

        self.assertTrue(filecmp.cmp(asmcomp, binout, shallow=False))

    def test_default_int_with_comments(self): #int with no indicated number with comment instead of newline
        asmin = "./testasmfiles/defaultint_with_comment.asm"
        binout = "./testasmfiles/defaultint_with_comment.bin"
        subprocess.run([sys.executable, "asm4380.py", asmin])
        asmcomp = "./testbinfiles/defaultint.bin"

        self.assertTrue(filecmp.cmp(asmcomp, binout, shallow=False))


if __name__ == "__main__":
    unittest.main()