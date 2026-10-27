
import subprocess
import sys
from pathlib import Path
 
from asm4380 import process
 
SCRIPT = Path(__file__).parent / "asm4380.py"
USAGE = "USAGE: python3 asm4380.py inputFile.asm\n"
 
# With no data section, the first instruction sits at address 4,
# right after the 4-byte header.
HEADER = bytes([4, 0, 0, 0])
TRP0 = bytes([31, 0, 0, 0, 0, 0, 0, 0])
 
 
def asm(tmp_path, source, name="program.asm"):
    """Assemble `source` with process() and return the bytes (header included)."""
    path = tmp_path / name
    path.write_text(source)
    return b"".join(process(str(path)))
 
 
def cli(tmp_path, source=None, name="program.asm", args=None):
    """Run asm4380.py as a subprocess, the way a user would."""
    if source is not None:
        (tmp_path / name).write_text(source)
    return subprocess.run(
        [sys.executable, str(SCRIPT)] + (args if args is not None else [name]),
        capture_output=True, text=True, cwd=tmp_path,
    )
 
 
def le(n, size=4, signed=False):
    return n.to_bytes(size, byteorder="little", signed=signed)
 
 
# --- Instruction encoding ---------------------------------------
 
def test_trp0(tmp_path):
    """TRP #0 -> opcode 31, three DC bytes, 4-byte immediate 0."""
    assert asm(tmp_path, "\tTRP #0\n") == HEADER + TRP0
 
 
def test_movi(tmp_path):
    """MOVI R1, #5 -> opcode 8, RD=1, two DC bytes, immediate 5."""
    assert asm(tmp_path, "\tMOVI R1, #5\n") == HEADER + bytes([8, 1, 0, 0, 5, 0, 0, 0])
 
 
def test_add(tmp_path):
    """ADD R3, R1, R2 -> opcode 18, three register bytes, immediate 0."""
    assert asm(tmp_path, "\tADD R3, R1, R2\n") == HEADER + bytes([18, 3, 1, 2, 0, 0, 0, 0])
 
 
def test_negative_imm(tmp_path):
    """Immediates are signed 4-byte little-endian values."""
    joined = asm(tmp_path, "\tADDI R2, R1, #-5\n")
    assert joined == HEADER + bytes([19, 2, 1, 0]) + le(-5, signed=True)
 
 
def test_order(tmp_path):
    """Instructions come out back to back, in file order."""
    joined = asm(tmp_path, "\tMOVI R1, #5\n\tTRP #0\n")
    assert joined == HEADER + bytes([8, 1, 0, 0, 5, 0, 0, 0]) + TRP0
 
 
# --- Case, comments, blank lines --------------------------------
 
def test_case_insensitive(tmp_path):
    """Operators, registers, and directives ignore case."""
    assert asm(tmp_path, "\taDd r13, R1, r2\n", "a.asm") == asm(tmp_path, "\tADD R13, R1, R2\n", "b.asm")
    assert asm(tmp_path, "X\t.int #7\n\tTRP #0\n", "c.asm") == asm(tmp_path, "X\t.INT #7\n\tTRP #0\n", "d.asm")
 
 
def test_comments_blanks(tmp_path):
    """Comment lines, inline comments, and blank lines add nothing."""
    messy = "; full line\n\n   \n\tMOVI R1, #5 ; inline\n"
    assert asm(tmp_path, messy, "a.asm") == asm(tmp_path, "\tMOVI R1, #5\n", "b.asm")
 
    # The spec's own example has no space before the ';'
    joined = asm(tmp_path, "A_LABEL\t.BYT #245;a comment\n", "c.asm")
    assert joined[4:5] == bytes([245])
 
 
# --- Data section, labels, header -------------------------------
 
def test_data(tmp_path):
    """.INT is 4 bytes, .BYT is 1, and a missing operand defaults to 0."""
    source = (
        "A\t.INT #10\n"      # address 4-7
        "B\t.BYT #65\n"      # address 8
        "\t.BYT\n"           # address 9
        "C\t.INT\n"          # address 10-13
        "D\t.INT #-12\n"     # address 14-17
        "\tTRP #0\n"
    )
    joined = asm(tmp_path, source)
    assert joined[4:18] == le(10) + bytes([65, 0]) + le(0) + le(-12, signed=True)
 
 
def test_header(tmp_path):
    """The header holds the address of the first instruction (after the data)."""
    joined = asm(tmp_path, "A\t.INT #10\nB\t.BYT #65\n\tTRP #0\n")
    assert joined[:4] == le(9)
 
 
def test_data_label(tmp_path):
    """A label operand becomes the label's address, not its contents."""
    joined = asm(tmp_path, "COUNT\t.INT #10\n\tLDR R1, COUNT\n")
    assert joined[8:16] == bytes([11, 1, 0, 0]) + le(4)
 
 
def test_forward_jmp(tmp_path):
    source = (
        "\tMOVI R1, #1\n"      # address 4
        "\tJMP SKIP\n"         # address 12
        "\tMOVI R1, #99\n"     # address 20
        "SKIP\tTRP #0\n"       # address 28
    )
    assert asm(tmp_path, source)[12:20] == bytes([1, 0, 0, 0]) + le(28)
 
 
def test_backward_jmp(tmp_path):
    joined = asm(tmp_path, "START\tMOVI R1, #1\n\tJMP START\n")
    assert joined[12:20] == bytes([1, 0, 0, 0]) + le(4)
 
 
def test_char_literals(tmp_path):
    """Plain characters, #codes, special characters, and all seven escapes."""
    cases = [
        ("'A'", 65), ("'#'", 35), ("';'", 59), ("','", 44), ("' '", 32), ("#83", 83),
        (r"'\t'", 9), (r"'\n'", 10), (r"'\\'", 92), (r"'\''", 39),
        (r"'\"'", 34), (r"'\r'", 13), (r"'\b'", 8),
    ]
    for literal, expected in cases:
        joined = asm(tmp_path, f"X\t.BYT {literal}\n\tTRP #0\n")
        assert joined[4:5] == bytes([expected]), literal
 
 
# --- Req8 / Req9: assembler errors ------------------------------
 
def test_error_output(tmp_path):
    """Message names the failing line, exit code is 2, no .bin is written."""
    result = cli(tmp_path, "\tTRP #0\n\tFOO\n")
    assert result.returncode == 2
    assert result.stdout == "Assembler error encountered on line 2!\n"
    assert not (tmp_path / "program.bin").exists()
 
 
def test_operand_errors(tmp_path):
    """Wrong operand count/type/range is reported on its own line."""
    cases = [
        ("\tTRP #0\n\tADD R1, R2\n", 2),        # too few operands
        ("\tMOV R1, R2, R3\n", 1),              # too many operands
        ("\tMOV R1, R99\n", 1),                 # invalid register
        ("\tMOVI R1, 5\n", 1),                  # immediate missing '#'
        ("\tADD R1 R2 R3\n", 1),                # missing commas
        ("\tMOVI R1, #99999999999\n", 1),       # immediate out of range
        ("\tTRP #5\n", 1),                      # not a defined trap
        ("X\t.BYT #256\n", 1),                  # .BYT out of range
        ("X\t.INT #2147483648\n", 1),           # .INT out of range
    ]
    for source, line in cases:
        result = cli(tmp_path, source)
        assert result.returncode == 2, source
        assert result.stdout == f"Assembler error encountered on line {line}!\n", source
 
 
def test_label_errors(tmp_path):
    """Label and section rules are enforced."""
    cases = [
        ("\tTRP #0\nX\t.INT #5\n", 2),          # directive in the code section
        ("ORPHAN\n\tTRP #0\n", 1),              # label alone on a line
        ("_bad\t.INT #5\n", 1),                 # label can't start with '_'
        ("A\t.INT #1\nA\t.INT #2\n", 2),        # duplicate label
        ("   LOOP ADDI R1, R1, #1\n", 1),       # indented, so not a label
        ("X\t.FOO #5\n", 1),                    # unknown directive
    ]
    for source, line in cases:
        result = cli(tmp_path, source)
        assert result.returncode == 2, source
        assert result.stdout == f"Assembler error encountered on line {line}!\n", source
 
 
def test_missing_label(tmp_path):
    """Req9: an undefined label is reported on the last line of the file."""
    result = cli(tmp_path, "\tJMP NOWHERE\n\tTRP #0\n\n; end\n")
    assert result.returncode == 2
    assert result.stdout == "Assembler error encountered on line 4!\n"
 
 
def test_label_case(tmp_path):
    """Labels are case sensitive: 'skip' does not match 'SKIP'."""
    result = cli(tmp_path, "\tJMP skip\nSKIP\tTRP #0\n")
    assert result.returncode == 2
    assert result.stdout == "Assembler error encountered on line 2!\n"
 
 
# --- Req6 / Req7: command line and output file ------------------
 
def test_cli_success(tmp_path):
    """Exit 0, no output, and gradebook.asm produces gradebook.bin."""
    result = cli(tmp_path, "\tTRP #0\n", name="gradebook.asm")
    assert result.returncode == 0
    assert result.stdout == ""
    assert (tmp_path / "gradebook.bin").read_bytes() == HEADER + TRP0
 
 
def test_cli_usage(tmp_path):
    """Missing, extra, wrongly-named, or nonexistent input prints usage, exit 1."""
    (tmp_path / "program.asm").write_text("\tTRP #0\n")
    (tmp_path / "program.txt").write_text("\tTRP #0\n")
    cases = [[], ["program.asm", "extra"], ["program.txt"], ["nope.asm"]]
    for args in cases:
        result = cli(tmp_path, args=args)
        assert result.returncode == 1, args
        assert result.stdout == USAGE, args
        
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
