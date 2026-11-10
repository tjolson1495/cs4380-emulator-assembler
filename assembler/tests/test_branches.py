import unittest
import filecmp
import subprocess
import sys
from pathlib import Path

TESTS = Path(__file__).resolve().parent
ASM_DIR = str(TESTS / "fixtures" / "asm")
EXPECTED_DIR = str(TESTS / "fixtures" / "expected")
ASM_SCRIPT = str(TESTS.parent / "asm4380.py")

class TestAssembler(unittest.TestCase):
    def test_jmr(self):
        asmin = ASM_DIR + "/JMRTest.asm"
        binout = ASM_DIR + "/JMRTest.bin"
        subprocess.run([sys.executable, ASM_SCRIPT, asmin])
        asmcomp = EXPECTED_DIR + "/JMRTest.bin"

        self.assertTrue(filecmp.cmp(asmcomp, binout, shallow=False))

    def test_BGT_no_jump(self):
        asmin = ASM_DIR + "/BGT_no_jump.asm"
        binout = ASM_DIR + "/BGT_no_jump.bin"
        subprocess.run([sys.executable, ASM_SCRIPT, asmin])
        asmcomp = EXPECTED_DIR + "/BGTNoJump.bin"

        self.assertTrue(filecmp.cmp(asmcomp, binout, shallow=False))

    def test_BLT_no_jump(self):
        asmin = ASM_DIR + "/BLT_no_jump.asm"
        binout = ASM_DIR + "/BLT_no_jump.bin"
        subprocess.run([sys.executable, ASM_SCRIPT, asmin])
        asmcomp = EXPECTED_DIR + "/BLTNoJump.bin"

        self.assertTrue(filecmp.cmp(asmcomp, binout, shallow=False))

    def test_BNZ_no_jump(self):
        asmin = ASM_DIR + "/BNZ_no_jump.asm"
        binout = ASM_DIR + "/BNZ_no_jump.bin"
        subprocess.run([sys.executable, ASM_SCRIPT, asmin])
        asmcomp = EXPECTED_DIR + "/BNZNoJump.bin"

        self.assertTrue(filecmp.cmp(asmcomp, binout, shallow=False))

    def test_BNZ_with_jump(self):
        asmin = ASM_DIR + "/BNZ_with_jump.asm"
        binout = ASM_DIR + "/BNZ_with_jump.bin"
        subprocess.run([sys.executable, ASM_SCRIPT, asmin])
        asmcomp = EXPECTED_DIR + "/BNZJump.bin"

        self.assertTrue(filecmp.cmp(asmcomp, binout, shallow=False))

    def test_BRZ_no_jump(self):
        asmin = ASM_DIR + "/BRZ_no_jump.asm"
        binout = ASM_DIR + "/BRZ_no_jump.bin"
        subprocess.run([sys.executable, ASM_SCRIPT, asmin])
        asmcomp = EXPECTED_DIR + "/BRZNoJump.bin"

        self.assertTrue(filecmp.cmp(asmcomp, binout, shallow=False))

    def test_BRZ_with_jump(self):
        asmin = ASM_DIR + "/BRZ_with_jump.asm"
        binout = ASM_DIR + "/BRZ_with_jump.bin"
        subprocess.run([sys.executable, ASM_SCRIPT, asmin])
        asmcomp = EXPECTED_DIR + "/BRZJump.bin"

        self.assertTrue(filecmp.cmp(asmcomp, binout, shallow=False))

    def test_ex_asm(self): #example.asm from assembler mark 2 in canvas
        asmin = ASM_DIR + "/example.asm"
        binout = ASM_DIR + "/example.bin"
        subprocess.run([sys.executable, ASM_SCRIPT, asmin])
        asmcomp = EXPECTED_DIR + "/example.bin"

        self.assertTrue(filecmp.cmp(asmcomp, binout, shallow=False))

    def test_minimal(self): #trap 0 with no comments
        asmin = ASM_DIR + "/minimal_without_comment.asm"
        binout = ASM_DIR + "/minimal_without_comment.bin"
        subprocess.run([sys.executable, ASM_SCRIPT, asmin])
        asmcomp = EXPECTED_DIR + "/minimal.bin"

        self.assertTrue(filecmp.cmp(asmcomp, binout, shallow=False))

    def test_minimal_with_comments(self): #trap 0 with comment
        asmin = ASM_DIR + "/minimal_with_comment.asm"
        binout = ASM_DIR + "/minimal_with_comment.bin"
        subprocess.run([sys.executable, ASM_SCRIPT, asmin])
        asmcomp = EXPECTED_DIR + "/minimal.bin"

        self.assertTrue(filecmp.cmp(asmcomp, binout, shallow=False))

    def test_default_int(self): #int with no indicated number and then a trap 0
        asmin = ASM_DIR + "/defaultint_without_comment.asm"
        binout = ASM_DIR + "/defaultint_without_comment.bin"
        subprocess.run([sys.executable, ASM_SCRIPT, asmin])
        asmcomp = EXPECTED_DIR + "/defaultint.bin"

        self.assertTrue(filecmp.cmp(asmcomp, binout, shallow=False))

    def test_default_int_with_comments(self): #int with no indicated number with comment instead of newline
        asmin = ASM_DIR + "/defaultint_with_comment.asm"
        binout = ASM_DIR + "/defaultint_with_comment.bin"
        subprocess.run([sys.executable, ASM_SCRIPT, asmin])
        asmcomp = EXPECTED_DIR + "/defaultint.bin"

        self.assertTrue(filecmp.cmp(asmcomp, binout, shallow=False))


if __name__ == "__main__":
    unittest.main()