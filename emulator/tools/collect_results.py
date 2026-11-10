#!/usr/bin/env python3
"""Run a folder of 4380 programs on every cache type and collect the memory cycles.
 
For each program (.asm files are assembled first; .bin files are run as they are)
it runs the emulator with no cache, direct mapped, fully associative and,
optionally, 2-way set associative, then prints a table and writes a CSV file:
one row per program, one column per cache type, and in each cell the final value
of mem_cycle_cntr (the number printed by TRP 0). Open the CSV in LibreOffice Calc
or Excel and save it as .ods or .xlsx if you want.
 
It also checks that the program's own output is identical for every cache type.
A cache must never change what a program prints; if it does, the cycle counts
for that program cannot be trusted and a warning is printed (the CSV itself holds
only the cycle counts).
 
Examples (run from emulator/tools):
    python3 collect_results.py ~/professor_programs --label 64x16
    python3 collect_results.py ~/professor_programs --label 32x32 --two-way
    python3 collect_results.py ~/professor_programs --emulator ../emu4380 --out run1.csv
 
Geometry (lines x block size) is set when the emulator is compiled, so for Part 5
rebuild the emulator with 32 lines of 32-byte blocks and run this again with
--label 32x32. The label is only written into the CSV and the default file name.
"""
import argparse
import csv
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path
 
HERE = Path(__file__).resolve().parent
CACHE_NAMES = {0: "No cache", 1: "Direct mapped", 2: "Fully associative", 3: "2-way set associative"}
CYCLES = re.compile(r"Total memory cycles:\s*(\d+)")
 
 
def natural_key(path):
    return [int(t) if t.isdigit() else t.lower() for t in re.split(r"(\d+)", path.name)]
 
 
def assemble(asm_file, assembler, work):
    """Assemble a copy of asm_file inside `work` (the assembler writes the .bin next to its input)."""
    copy = work / asm_file.name
    shutil.copy(asm_file, copy)
    result = subprocess.run([sys.executable, str(assembler), str(copy)],
                            capture_output=True, text=True)
    if result.returncode != 0:
        return None, "assembler error: " + (result.stdout + result.stderr).strip()
    return copy.with_suffix(".bin"), None
 
 
def run_once(emulator, binfile, cache, timeout):
    """Returns (cycles or None, program output, problem or None)."""
    try:
        r = subprocess.run([str(emulator), str(binfile), "-c", str(cache)],
                           capture_output=True, text=True, timeout=timeout,
                           stdin=subprocess.DEVNULL)
    except subprocess.TimeoutExpired:
        return None, "", "timed out after %ds" % timeout
 
    found = CYCLES.search(r.stdout)
    if not found:
        return None, r.stdout, "no cycle count printed (exit code %d)" % r.returncode
 
    # The program's own output is everything except the final cycle message
    output = r.stdout[:found.start()].replace("Execution completed.", "").strip()
    return int(found.group(1)), output, None
 
 
def main():
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("programs", type=Path, help="folder holding the .asm and/or .bin programs")
    p.add_argument("--emulator", type=Path, default=HERE.parent / "emu4380")
    p.add_argument("--assembler", type=Path, default=HERE.parent.parent / "assembler" / "asm4380.py")
    p.add_argument("--label", default="run", help="cache geometry label for the output, e.g. 64x16")
    p.add_argument("--two-way", action="store_true", help="also run cache type 3 (extra credit)")
    p.add_argument("--timeout", type=int, default=60, help="seconds allowed per run (default 60)")
    p.add_argument("--out", type=Path, help="CSV file to write (default results_<label>.csv)")
    args = p.parse_args()
 
    if not args.programs.is_dir():
        sys.exit("Not a folder: %s" % args.programs)
    if not args.emulator.is_file():
        sys.exit("Emulator not found: %s (build it with make, or use --emulator)" % args.emulator)
 
    files = sorted([f for f in args.programs.iterdir() if f.suffix in (".asm", ".bin")], key=natural_key)
    # If a program exists as both .asm and .bin, trust the .asm (it is assembled fresh)
    asm_stems = {f.stem for f in files if f.suffix == ".asm"}
    files = [f for f in files if f.suffix == ".asm" or f.stem not in asm_stems]
    if not files:
        sys.exit("No .asm or .bin files in %s" % args.programs)
 
    caches = [0, 1, 2] + ([3] if args.two_way else [])
    rows = []
    problems = []
 
    with tempfile.TemporaryDirectory() as tmp:
        work = Path(tmp)
 
        for f in files:
            name = f.stem
            if f.suffix == ".asm":
                binfile, err = assemble(f, args.assembler, work)
                if err:
                    problems.append("%s: %s" % (name, err))
                    continue
            else:
                binfile = f
 
            cycles, outputs = {}, {}
            for c in caches:
                n, out, err = run_once(args.emulator, binfile, c, args.timeout)
                if err:
                    problems.append("%s (%s): %s" % (name, CACHE_NAMES[c], err))
                cycles[c], outputs[c] = n, out
 
            same_output = len({outputs[c] for c in caches if cycles[c] is not None}) <= 1
            if not same_output:
                problems.append("%s: program output differs between cache types -- "
                                "a cache is changing the program's results" % name)
 
            rows.append((name, cycles, same_output))
 
    # ---- build the table: programs down the rows, cache types across the columns ----
    header = ["Program"] + [CACHE_NAMES[c] for c in caches]
    table = [[name] + ["" if cycles[c] is None else cycles[c] for c in caches]
             for name, cycles, same in rows]
 
    out = args.out or Path("results_%s.csv" % args.label)
    with open(out, "w", newline="") as fh:
        w = csv.writer(fh)
        w.writerow(header)
        w.writerows(table)
 
    # ---- console view ----
    widths = [max(len(str(r[i])) for r in [header] + table) for i in range(len(header))]
    for r in [header] + table:
        print("  ".join(str(v).rjust(widths[i]) if i else str(v).ljust(widths[i]) for i, v in enumerate(r)))
 
    print("\nCells are the final mem_cycle_cntr value. Geometry label: %s" % args.label)
    print("Wrote %s" % out)
 
    if problems:
        print("\nProblems:")
        for msg in problems:
            print("  - " + msg)
        sys.exit(1)
 
 
if __name__ == "__main__":
    main()
 