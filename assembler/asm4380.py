import os
import re
import sys
from enum import Enum 
 
class AssemblerError(Exception):
    def __init__(self, line=0):
        super().__init__(line)
        self.line = line
 
class OPP(Enum):
    JMP  = 1
    JMR  = 2
    BNZ  = 3
    BGT  = 4
    BLT  = 5
    BRZ  = 6
    MOV  = 7
    MOVI = 8
    LDA  = 9
    STR  = 10
    LDR  = 11
    STB  = 12
    LDB  = 13
    ISTR = 14
    ILDR = 15
    ISTB = 16
    ILDB = 17
    ADD  = 18
    ADDI = 19
    SUB  = 20
    SUBI = 21
    MUL  = 22
    MULI = 23
    DIV  = 24
    SDIV = 25
    DIVI = 26
    CMP  = 29
    CMPI = 30
    TRP  = 31
        
class REG(Enum):
    R0 = 0
    R1 = 1
    R2 = 2
    R3 = 3
    R4 = 4
    R5 = 5
    R6 = 6
    R7 = 7
    R8 = 8
    R9 = 9
    R10 = 10
    R11 = 11
    R12 = 12
    R13 = 13
    R14 = 14
    R15 = 15
    PC = 16
    SL = 17
    SB = 18
    SP = 19
    FP = 20
    HP = 21
    
registers = [member.name for member in REG]
 
# Operand types each operator expects, in the order they are written
FORMS = {
    "JMP":  ["label"],
    "JMR":  ["reg"],
    "BNZ":  ["reg", "label"],
    "BGT":  ["reg", "label"],
    "BLT":  ["reg", "label"],
    "BRZ":  ["reg", "label"],
    "MOV":  ["reg", "reg"],
    "MOVI": ["reg", "imm"],
    "LDA":  ["reg", "label"],
    "STR":  ["reg", "label"],
    "LDR":  ["reg", "label"],
    "STB":  ["reg", "label"],
    "LDB":  ["reg", "label"],
    "ISTR": ["reg", "reg"],
    "ILDR": ["reg", "reg"],
    "ISTB": ["reg", "reg"],
    "ILDB": ["reg", "reg"],
    "ADD":  ["reg", "reg", "reg"],
    "ADDI": ["reg", "reg", "imm"],
    "SUB":  ["reg", "reg", "reg"],
    "SUBI": ["reg", "reg", "imm"],
    "MUL":  ["reg", "reg", "reg"],
    "MULI": ["reg", "reg", "imm"],
    "DIV":  ["reg", "reg", "reg"],
    "SDIV": ["reg", "reg", "reg"],
    "DIVI": ["reg", "reg", "imm"],
    "CMP":  ["reg", "reg", "reg"],
    "CMPI": ["reg", "reg", "imm"],
    "TRP":  ["imm"],
}
 
TRAPS = (0, 1, 2, 3, 4, 98)
BRANCHES = (3, 4, 5, 6)
 
ESCAPES = {'t': 9, '\\': 92, 'n': 10, "'": 39, '"': 34, 'r': 13, 'b': 8}
 
def validate_label(label:str) -> bool:
    
    if not label:
        return False
    
    # First character must be alpha numeric
    if not (label[0].isascii() and label[0].isalnum()):
        return False
    
    for c in label:
        
        # Check alpha numeric, $ and _
        if not (c.isascii() and c.isalnum()) and c not in "$_":
            return False 
            
    return True
 
def char_span(s: str, i: int):
    
    # End of a character literal starting at s[i] ('a' or '\n'), else None
    n = 4 if s[i+1:i+2] == '\\' else 3
    
    if s[i+n-1:i+n] == "'":
        return i + n
    
    return None
 
def strip_comment(line: str) -> str:
    
    i = 0
    
    while i < len(line):
        
        if line[i] == ';':
            return line[:i]
        
        # A ';' inside a character literal is not a comment
        if line[i] == "'" and char_span(line, i):
            i = char_span(line, i)
        else:
            i += 1
            
    return line
 
def split_operands(text: str) -> list[str]:
    
    if not text:
        return []
    
    operands = []
    current = ""
    i = 0
    
    while i < len(text):
        
        # A ',' inside a character literal does not separate operands
        if text[i] == "'" and char_span(text, i):
            end = char_span(text, i)
            current += text[i:end]
            i = end
        elif text[i] == ',':
            operands.append(current.strip())
            current = ""
            i += 1
        else:
            current += text[i]
            i += 1
            
    operands.append(current.strip())
    
    return operands
 
def parse_number(token: str):
    
    # #45 -> 45, anything else -> None
    if re.fullmatch(r"#-?[0-9]+", token):
        return int(token[1:])
    
    return None
 
def parse_literal(token: str):
    
    value = parse_number(token)
    
    if value is not None:
        return value
    
    # 'a' -> 97
    if len(token) == 3 and token[0] == token[2] == "'":
        if token[1] not in "'\\" and 32 <= ord(token[1]) < 127:
            return ord(token[1])
        
    # '\n' -> 10
    if len(token) == 4 and token[0] == token[3] == "'" and token[1] == '\\':
        return ESCAPES.get(token[2])
        
    return None
 
def process_directive(directive: str, operands: list[str], output: list[bytes]):
    
    directive = directive.upper()
    value = 0
    
    if directive not in (".INT", ".BYT") or len(operands) > 1:
        raise AssemblerError()
    
    if operands:
        if directive == ".INT":
            value = parse_number(operands[0])
        else:
            value = parse_literal(operands[0])
            
        if value is None:
            raise AssemblerError()
    
    if (directive == ".INT"):
        if not -2**31 <= value < 2**31:
            raise AssemblerError()
        
        output.append(value.to_bytes(4, byteorder='little', signed=True))
    else:
        if not 0 <= value < 256:
            raise AssemblerError()
        
        output.append(value.to_bytes(1, byteorder='little'))
        
def process_instruction(operator: str, operands: list[str], output: list[bytes], pending: list):
        
    operator = operator.upper()
    
    if operator not in FORMS or len(operands) != len(FORMS[operator]):
        raise AssemblerError()
    
    reg_codes = []
    number = 0
    value = bytes(4)
    
    for kind, t in zip(FORMS[operator], operands):
                    
        if kind == "reg":
            if t.upper() not in registers:
                raise AssemblerError()
            
            reg_codes.append(REG[t.upper()].value.to_bytes(1, byteorder='little'))
        elif kind == "imm": 
            number = parse_literal(t)
            
            if number is None or not -2**31 <= number < 2**31:
                raise AssemblerError()
            
            value = number.to_bytes(4, byteorder='little', signed=True)
        else:
            if not validate_label(t):
                raise AssemblerError()
            
            # Label, filled in once every label's address is known
            pending.append((len(output) + 4, t, OPP[operator].value in BRANCHES))
            
    if operator == "TRP" and number not in TRAPS:
        raise AssemblerError()
            
    opp_code = OPP[operator].value.to_bytes(1, byteorder='little')
    
    output.append(opp_code)
    output.extend(reg_codes)
    output.extend([bytes(1)] * (3 - len(reg_codes)))     # Don't care operands
    output.append(value)
 
def process(filename:str ) -> list[bytes]:
 
    output = [bytes(4)]     # Holds the address of the first instruction
    labels = {}
    pending = []
    code_start = None
    number = 0
 
    with open(filename, encoding="utf-8", errors="replace") as file:
        
        for number, f in enumerate(file, start=1):
            
            try:
                
                line = strip_comment(f)
                
                if not line.strip():
                    continue
                
                address = sum(len(o) for o in output)
                tokens = line.split(None, 1)
                            
                # Determine if line defines a label
                if line[0] not in " \t":
                    label = tokens[0]
                    
                    if not validate_label(label) or label in labels or len(tokens) < 2:
                        raise AssemblerError()
                    
                    labels[label] = address
                    tokens = tokens[1].split(None, 1)
                    
                operator = tokens[0]
                operands = split_operands(tokens[1].strip() if len(tokens) > 1 else "")
    
                if operator[0] == '.':
                    
                    # Directives are only allowed before the first instruction
                    if code_start is not None:
                        raise AssemblerError()
                    
                    process_directive(operator, operands, output)
                else:
                    if code_start is None:
                        code_start = address
                        
                    process_instruction(operator, operands, output, pending)
                    
            except AssemblerError as e:
                e.line = number
                raise
                
    for position, label, branch in pending:
        
        # A missing label is only detectable at the end of the file
        if label not in labels:
            raise AssemblerError(number)
        
        # branch is boolean type
        if branch and labels[label] < code_start:
            raise AssemblerError(number)
        
        output[position] = labels[label].to_bytes(4, byteorder='little')
 
    if code_start is None:
        code_start = sum(len(o) for o in output)
        
    output[0] = code_start.to_bytes(4, byteorder='little')
    print(os.path.splitext(filename)[0])
    write(output, os.path.splitext(filename)[0] + ".bin")
                          
    return output
 
def write(output, filename):
    with open(filename, "wb") as file:
        for o in output:
            file.write(o)
            
if __name__ == "__main__":
    
    if len(sys.argv) != 2 or not sys.argv[1].endswith(".asm") or not os.path.isfile(sys.argv[1]):
        print("USAGE: python3 asm4380.py inputFile.asm")
        sys.exit(1)
        
    try:
        process(sys.argv[1])
    except AssemblerError as e:
        print(f"Assembler error encountered on line {e.line}!")
        sys.exit(2)
        
    sys.exit(0)