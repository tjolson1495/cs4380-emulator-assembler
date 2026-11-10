#!/usr/bin/env bash
#
# Assembles every T*.asm in this folder and runs it on cache types 0 (none),
# 1 (direct mapped) and 2 (fully associative), comparing the memory-cycle count
# with the expected value (see results.md for what each program does).
#
# Usage:  ./run_cache_tests.sh [64x16|32x32] [path/to/emu4380]
#
#   geometry  must match how the emulator was compiled (default 64x16;
#             32x32 needs -DCACHE_LINES=32 -DCACHE_BLOCK_SIZE=32)
#   emulator  defaults to ../../emu4380 (the `make` output in emulator/)
#
# Works from any directory. Nothing is written into the repository: programs
# are assembled in a temporary folder that is removed afterwards.

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
GEOM="${1:-64x16}"
EMU="${2:-$HERE/../../emu4380}"
ASM="$HERE/../../../assembler/asm4380.py"

if [ "$GEOM" != "64x16" ] && [ "$GEOM" != "32x32" ]; then
    echo "Geometry must be 64x16 or 32x32"
    exit 1
fi

if [ ! -x "$EMU" ]; then
    echo "Emulator not found or not executable: $EMU"
    echo "Build it with 'make' in emulator/, or pass its path as the second argument."
    exit 1
fi

if [ ! -f "$ASM" ]; then
    echo "Assembler not found: $ASM"
    exit 1
fi

# Expected cycles: "program none direct_mapped fully_associative"
if [ "$GEOM" = "64x16" ]; then
    EXPECTED="
T01_minimal           10    16    16
T02_straight_line     80    86    86
T03_loop_countdown    2020  446   446
T04_array_sum         978   326   326
T05_stride16_sweep    3792  1308  1308
T06_conflict_pingpong 3916  2304  876
T07_lru_sweep         11470 3563  5061
T08_dirty_writeback   1426  1520  400
T09_byte_access       7504  1620  1620
T10_unaligned_span    558   211   211
T11_array_copy        2224  668   668
T12_correctness       998   638   428
"
else
    EXPECTED="
T01_minimal           10    24    24
T02_straight_line     80    82    82
T03_loop_countdown    2020  448   448
T04_array_sum         978   296   296
T05_stride16_sweep    3792  1182  1182
T06_conflict_pingpong 3916  3120  876
T07_lru_sweep         11470 3651  4487
T08_dirty_writeback   1426  2152  392
T09_byte_access       7504  1578  1578
T10_unaligned_span    558   209   209
T11_array_copy        2224  614   614
T12_correctness       998   806   476
"
fi

WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

pass=0
fail=0

while read -r name none dm fa; do
    [ -z "$name" ] && continue

    # The assembler writes the .bin next to its input, so assemble a copy
    cp "$HERE/$name.asm" "$WORK/"
    if ! python3 "$ASM" "$WORK/$name.asm"; then
        echo "FAIL  $name: assembler error"
        fail=$((fail + 3))
        continue
    fi

    cache=0
    for want in "$none" "$dm" "$fa"; do
        out=$("$EMU" "$WORK/$name.bin" -c "$cache" 2>&1)
        got=$(echo "$out" | grep -o 'Total memory cycles: *[0-9]*' | grep -o '[0-9]*$')

        if [ "$got" = "$want" ]; then
            echo "PASS  $name  -c $cache  ($got)"
            pass=$((pass + 1))
        else
            echo "FAIL  $name  -c $cache  expected $want, got ${got:-nothing}"
            fail=$((fail + 1))
        fi
        cache=$((cache + 1))
    done
done <<< "$EXPECTED"

echo
echo "Geometry $GEOM: $pass passed, $fail failed"
[ "$fail" -eq 0 ]
