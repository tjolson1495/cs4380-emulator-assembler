import pytest
from asm4380 import process, AssemblerError
 
 
def asm(tmp_path, text):
    """Write source to a temp .asm file, assemble it, return the bytes."""
    src = tmp_path / "prog.asm"
    src.write_text(text)
    return b"".join(process(str(src)))
 
 
def base(i):
    return 4 + 8 * i
 
 
def imm_at(code, i, signed=False):
    off = base(i) + 4
    return int.from_bytes(code[off:off + 4], "little", signed=signed)
 
 
def test_istr(tmp_path):
    code = asm(tmp_path, "    ISTR R1, R2\n")
    b = base(0)
    assert code[b] == 14
    assert code[b + 1] == 1     # RS
    assert code[b + 2] == 2     # RG
    assert code[b + 3] == 0     # unused operand
    assert imm_at(code, 0) == 0
 
 
def test_ildr(tmp_path):
    code = asm(tmp_path, "    ILDR R1, R2\n")
    b = base(0)
    assert code[b] == 15
    assert code[b + 1] == 1     # RD
    assert code[b + 2] == 2     # RG
    assert code[b + 3] == 0
 
 
def test_cmp(tmp_path):
    code = asm(tmp_path, "    CMP R3, R1, R2\n")
    b = base(0)
    assert code[b] == 29
    assert code[b + 1] == 3     # RD
    assert code[b + 2] == 1     # RS1
    assert code[b + 3] == 2     # RS2
    assert imm_at(code, 0) == 0
 
 
def test_cmpi(tmp_path):
    code = asm(tmp_path, "    CMPI R3, R1, #-10\n")
    b = base(0)
    assert code[b] == 30
    assert code[b + 1] == 3     # RD
    assert code[b + 2] == 1     # RS1
    assert code[b + 3] == 0     # unused operand
    assert imm_at(code, 0, signed=True) == -10
 
 
def test_bnz_label(tmp_path):
    src = (
        "    BNZ R1, skip\n"      # offset 4
        "    ADDI R2, R2, #1\n"   # offset 12
        "skip TRP #0\n"           # offset 20
    )
    code = asm(tmp_path, src)
    b = base(0)
    assert code[b] == 3
    assert code[b + 1] == 1
    assert imm_at(code, 0) == 20
 
 
# --- instruction-label rule for branches ---
 
def test_branch_to_data_label_fails(tmp_path):
    src = (
        "val .INT #5\n"
        "    BNZ R1, val\n"
        "    TRP #0\n"
    )
    with pytest.raises(AssemblerError):
        asm(tmp_path, src)
 
 
def test_branch_to_missing_label_fails(tmp_path):
    src = (
        "    BNZ R1, nowhere\n"
        "    TRP #0\n"
    )
    with pytest.raises(AssemblerError):
        asm(tmp_path, src)
 
 
def test_lda_to_data_label_still_ok(tmp_path):
    src = (
        "val .INT #5\n"
        "    LDA R1, val\n"
        "    TRP #0\n"
    )
    code = asm(tmp_path, src)
    # header (4) + .INT (4): LDA sits at offset 8, its immediate at 12,
    # and it should hold the address of "val" (offset 4)
    assert int.from_bytes(code[12:16], "little") == 4
 