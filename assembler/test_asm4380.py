import shutil
import subprocess
from pathlib import Path
 
TESTS_DIR = Path(__file__).resolve().parent
DATA_DIR = TESTS_DIR / "data"
 
 
def find_asm():
    # Req5 puts asm4380.py in an assembler/ folder at the repo root, so
    # search upward for it there (or directly in a parent folder)
    for parent in Path(__file__).resolve().parents:
        for candidate in (parent / "asm4380.py", parent / "assembler" / "asm4380.py"):
            if candidate.exists():
                return candidate
    raise FileNotFoundError("asm4380.py not found in any parent folder")
 
 
ASM = find_asm()
 
USAGE = "USAGE: python3 asm4380.py inputFile.asm"
 
 
# region Helpers
 
def run_asm(tmp_path, *args):
    return subprocess.run(
        ["python3", str(ASM), *args],
        cwd=tmp_path, capture_output=True, text=True, check=False,
    )
 
 
def assemble(tmp_path, file_name):
    # Copy first so the output never overwrites the expected .bin in data/
    shutil.copy(DATA_DIR / file_name, tmp_path / file_name)
    return run_asm(tmp_path, file_name)
 
 
def expected_bytes(bin_name):
    return (DATA_DIR / bin_name).read_bytes()
 
# endregion
 
 
# region Success tests
 
def test_single_trp0(tmp_path):
    """Lone TRP #0."""
    result = assemble(tmp_path, "trp0.asm")
 
    assert result.returncode == 0, result.stdout + result.stderr
    output = tmp_path / "trp0.bin"
    assert output.exists()
    assert output.read_bytes() == expected_bytes("trp0.bin")
 
 
def test_comments_blank_lines_and_whitespace(tmp_path):
    """Comments, blank lines, and mixed whitespace are ignored."""
    result = assemble(tmp_path, "trp0_comments_ws.asm")
 
    assert result.returncode == 0, result.stdout + result.stderr
    output = tmp_path / "trp0_comments_ws.bin"
    assert output.exists()
    assert output.read_bytes() == expected_bytes("trp0.bin")
 
 
def test_data_directives(tmp_path):
    """.INT and .BYT operand forms and defaults."""
    result = assemble(tmp_path, "data_directives.asm")
 
    assert result.returncode == 0, result.stdout + result.stderr
    output = tmp_path / "data_directives.bin"
    assert output.exists()
    assert output.read_bytes() == expected_bytes("data_directives.bin")
 
 
def test_labels_and_forward_reference(tmp_path):
    """Data and code labels, including a forward JMP."""
    result = assemble(tmp_path, "labels.asm")
 
    assert result.returncode == 0, result.stdout + result.stderr
    output = tmp_path / "labels.bin"
    assert output.exists()
    assert output.read_bytes() == expected_bytes("labels.bin")
 
 
def test_case_insensitivity(tmp_path):
    """Mixed-case directives, operators, and registers."""
    result = assemble(tmp_path, "case_insensitive.asm")
 
    assert result.returncode == 0, result.stdout + result.stderr
    output = tmp_path / "case_insensitive.bin"
    assert output.exists()
    assert output.read_bytes() == expected_bytes("labels.bin")
 
# endregion
 
 
# region Error tests
 
def test_directive_in_code_section(tmp_path):
    """Directive after the first instruction."""
    result = assemble(tmp_path, "directive_in_code.asm")
 
    assert result.returncode == 2
    assert result.stdout.strip() == "Assembler error encountered on line 2!"
 
 
def test_undefined_label(tmp_path):
    """Undefined label (Req9: reported on the last line of the file)."""
    result = assemble(tmp_path, "undefined_label.asm")
 
    assert result.returncode == 2
    assert result.stdout.strip() == "Assembler error encountered on line 3!"
 
 
def test_usage_no_argument(tmp_path):
    """No argument."""
    result = run_asm(tmp_path)
 
    assert result.returncode == 1
    assert result.stdout.strip() == USAGE
 
 
def test_usage_wrong_extension(tmp_path):
    """Input file without the .asm extension."""
    result = assemble(tmp_path, "not_assembly.txt")
 
    assert result.returncode == 1
    assert result.stdout.strip() == USAGE
 
# endregion
