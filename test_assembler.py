import shutil
import sys
import tempfile
import unittest
from pathlib import Path

# Find the assembler/ folder (Req5) so asm4380 can be imported from anywhere
for parent in Path(__file__).resolve().parents:
    if (parent / "assembler").is_dir():
        sys.path.insert(0, str(parent / "assembler"))
        break

from asm4380 import process

ASM_DIR = Path("./testasmfiles")
BIN_DIR = Path("./testbinfiles")


class TestAssembler(unittest.TestCase):

    def assert_assembles(self, asm_name, expected_name):
        # process() writes name.bin next to its input, so assemble a copy
        # in a temp folder to keep generated files out of testasmfiles
        with tempfile.TemporaryDirectory() as tmp:
            asm_copy = Path(tmp) / asm_name
            shutil.copy(ASM_DIR / asm_name, asm_copy)
            process(str(asm_copy))
            actual = asm_copy.with_suffix(".bin").read_bytes()

        self.assertEqual((BIN_DIR / expected_name).read_bytes(), actual)

    def test_ex_asm(self): #example.asm from assembler mark 2 in canvas
        self.assert_assembles("example.asm", "example.bin")

    def test_minimal(self): #trap 0 with no comments
        self.assert_assembles("minimal_without_comment.asm", "minimal.bin")

    def test_minimal_with_comments(self): #trap 0 with comment
        self.assert_assembles("minimal_with_comment.asm", "minimal.bin")

    def test_default_int(self): #int with no indicated number and then a trap 0
        self.assert_assembles("defaultint_without_comment.asm", "defaultint.bin")

    def test_default_int_with_comments(self): #int with no indicated number with comment instead of newline
        self.assert_assembles("defaultint_with_comment.asm", "defaultint.bin")


if __name__ == "__main__":
    unittest.main()
