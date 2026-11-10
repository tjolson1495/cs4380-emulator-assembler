# CS 4380 Project 3 Part 2: Cache Validation Tests

Twelve 4380 assembly programs with the expected number of memory cycles for no cache, a direct mapped cache and a fully associative cache, each at two cache geometries. The assembly files are submitted separately, one program per file.

## How to read the numbers

The value in each cell is the number printed by `TRP #0`: *Execution completed. Total memory cycles: N*. Programs that print a result use `TRP #1`, which prints no newline, so the output looks like `136Execution completed. Total memory cycles: 326`.

**Configurations** (cache geometry is set at compile time)

| Label | Cache lines | Block size | Used for |
|---|---|---|---|
| 64 x 16 | 64 | 16 bytes | Part 2 specification (Req7, Req8) |
| 32 x 32 | 32 | 32 bytes | Part 5 "What if?" |

**Cycle-counting rules these numbers assume** (so a teammate can tell a real bug from a different interpretation)

1. **No cache:** every memory access costs 8 cycles, except an instruction fetch, which costs 10 (8 for the first word, 2 for the immediate word; Req11).
2. **Cache hit:** 1 cycle for the access.
3. **Cache miss:** 1 cycle for the access, plus 8 for the first 4-byte word read from memory and 2 for each further word of the block(s) read (with 16-byte blocks, 4 words: one block costs 1+8+2x3 = 15 cycles and two blocks 1+8+2x7 = 23; with 32-byte blocks, 8 words: one block costs 1+8+2x7 = 23).
4. **Dirty eviction:** an additional 8 cycles for the first word written back plus 2 for each further word (16-byte blocks: one block is 14 cycles; the worst case, two blocks read and two written back, is 23+22 = 45. For 32-byte blocks one block is 22 cycles.)
5. **Instruction fetch with a cache** is two word accesses (the first word with the opcode and operands, then the immediate word), so a fetch that hits costs 2 cycles.
6. **One cache for instructions and data** (unified), **write-back** with **write-allocate** (a write miss reads the block first), LRU replacement in the fully associative cache.
7. A word access that crosses a block boundary touches both blocks within the same access (1 cycle for the access, burst costs combined).
8. Setting the initial PC from the first four bytes of the program is not counted (Req4). The cycle count printed by `TRP #0` includes the fetch of that `TRP #0` itself. Default memory size (131072 bytes).

## Summary of expected memory cycles

| Program | No cache | Direct mapped 64x16 | Fully assoc. 64x16 | Direct mapped 32x32 | Fully assoc. 32x32 |
|---|---:|---:|---:|---:|---:|
| T01_minimal | 10 | 16 | 16 | 24 | 24 |
| T02_straight_line | 80 | 86 | 86 | 82 | 82 |
| T03_loop_countdown | 2020 | 446 | 446 | 448 | 448 |
| T04_array_sum | 978 | 326 | 326 | 296 | 296 |
| T05_stride16_sweep | 3792 | 1308 | 1308 | 1182 | 1182 |
| T06_conflict_pingpong | 3916 | 2304 | 876 | 3120 | 876 |
| T07_lru_sweep | 11470 | 3563 | 5061 | 3651 | 4487 |
| T08_dirty_writeback | 1426 | 1520 | 400 | 2152 | 392 |
| T09_byte_access | 7504 | 1620 | 1620 | 1578 | 1578 |
| T10_unaligned_span | 558 | 211 | 211 | 209 | 209 |
| T11_array_copy | 2224 | 668 | 668 | 614 | 614 |
| T12_correctness | 998 | 638 | 428 | 806 | 476 |

## Program descriptions

### T01_minimal.asm

**What it does:** Smallest legal program: one TRP 0.

**Expected output:** No output besides the cycle message.

**Why the numbers look the way they do:** One instruction fetch and nothing else. Without a cache that is exactly the 10-cycle fetch from Req11. With a cache the first word of the instruction misses (a full block is read in) and the second word hits.

| No cache | DM 64x16 | FA 64x16 | DM 32x32 | FA 32x32 |
|---:|---:|---:|---:|---:|
| 10 | 16 | 16 | 24 | 24 |

### T02_straight_line.asm

**What it does:** Straight-line arithmetic with MOVI, ADD, ADDI, MULI and SUBI, then prints R3 with TRP 1.

**Expected output:** Prints 80  (((7+5)+30)*2)-4.

**Why the numbers look the way they do:** Eight instructions, no data accesses, so every cycle is an instruction fetch. 8 fetches x 10 = 80 without a cache. With a cache the cost is the cold misses on the few code blocks plus 2 cycles (two word hits) per remaining fetch.

| No cache | DM 64x16 | FA 64x16 | DM 32x32 | FA 32x32 |
|---:|---:|---:|---:|---:|
| 80 | 86 | 86 | 82 | 82 |

### T03_loop_countdown.asm

**What it does:** Countdown loop of 100 iterations (SUBI, BNZ).

**Expected output:** No output besides the cycle message.

**Why the numbers look the way they do:** 202 instruction fetches and no data accesses: 202 x 10 = 2020 without a cache. With either cache the loop body stays resident, so almost every fetch is two 1-cycle hits. This is the pure temporal-locality case, and the biggest speed-up in the suite.

| No cache | DM 64x16 | FA 64x16 | DM 32x32 | FA 32x32 |
|---:|---:|---:|---:|---:|
| 2020 | 446 | 446 | 448 | 448 |

### T04_array_sum.asm

**What it does:** Sums a 16-integer array (1..16) from the data segment using LDA and ILDR with a moving pointer.

**Expected output:** Prints 136.

**Why the numbers look the way they do:** 16 sequential word reads. With 16-byte blocks one miss brings in 4 integers, with 32-byte blocks 8, so the 32x32 configuration does better. Instruction and data share a cache, so the first data block also holds the start of the code.

| No cache | DM 64x16 | FA 64x16 | DM 32x32 | FA 32x32 |
|---:|---:|---:|---:|---:|
| 978 | 326 | 326 | 296 | 296 |

### T05_stride16_sweep.asm

**What it does:** Writes the values 0..31 to 32 addresses spaced 16 bytes apart (from address 16640), then reads them back and sums them.

**Expected output:** Prints 496.

**Why the numbers look the way they do:** With 16-byte blocks every access is to a different block (no spatial locality). With 32-byte blocks pairs of accesses share a block, so 32x32 is cheaper. Write misses allocate the block (write-allocate).

| No cache | DM 64x16 | FA 64x16 | DM 32x32 | FA 32x32 |
|---:|---:|---:|---:|---:|
| 3792 | 1308 | 1308 | 1182 | 1182 |

### T06_conflict_pingpong.asm

**What it does:** Alternates reads of two addresses exactly 1024 bytes apart (16640 and 17664) 50 times after storing 3 and 4 there.

**Expected output:** Prints 350  (50 x (3+4)).

**Why the numbers look the way they do:** 1024 = 64 lines x 16 bytes = 32 lines x 32 bytes, so both addresses map to the same line of the direct mapped cache in both configurations and evict each other on every access. The fully associative cache keeps both blocks, so after two cold misses everything hits. Largest direct mapped vs fully associative gap in the suite.

| No cache | DM 64x16 | FA 64x16 | DM 32x32 | FA 32x32 |
|---:|---:|---:|---:|---:|
| 3916 | 2304 | 876 | 3120 | 876 |

### T07_lru_sweep.asm

**What it does:** Three passes over 65 addresses spaced 16 bytes apart (from 16640), reads only.

**Expected output:** Prints 195  (3 x 65 reads).

**Why the numbers look the way they do:** 65 consecutive blocks is one more than a 64-line cache can hold. With strict LRU the fully associative cache always evicts the block that will be needed next, so every data access misses. The direct mapped cache only conflicts on the first and last block, so it WINS here. (For 32x32 the 65 locations cover 33 blocks versus 32 lines, same effect.)

| No cache | DM 64x16 | FA 64x16 | DM 32x32 | FA 32x32 |
|---:|---:|---:|---:|---:|
| 11470 | 3563 | 5061 | 3651 | 4487 |

### T08_dirty_writeback.asm

**What it does:** 20 rounds of storing a counter to two addresses 1024 bytes apart (16640 and 17664), then reading both back.

**Expected output:** Prints 40  (both locations hold 20).

**Why the numbers look the way they do:** In the direct mapped cache every store evicts the other, dirty, block, so each store pays a write-back plus a reload: the direct mapped total is actually HIGHER than with no cache at all for 64x16. The fully associative cache holds both lines dirty and never writes back. The correct final value also proves the evicted data reached memory.

| No cache | DM 64x16 | FA 64x16 | DM 32x32 | FA 32x32 |
|---:|---:|---:|---:|---:|
| 1426 | 1520 | 400 | 2152 | 392 |

### T09_byte_access.asm

**What it does:** Writes the bytes 0..63 to 64 consecutive addresses (ISTB) from 16640, then reads them back (ILDB) and sums them.

**Expected output:** Prints 2016  (0+1+...+63).

**Why the numbers look the way they do:** Byte accesses with perfect spatial locality: one miss per block, then 15 (or 31) hits. Without a cache each of the 128 byte accesses costs 8 cycles.

| No cache | DM 64x16 | FA 64x16 | DM 32x32 | FA 32x32 |
|---:|---:|---:|---:|---:|
| 7504 | 1620 | 1620 | 1578 | 1578 |

### T10_unaligned_span.asm

**What it does:** Stores a word at address 16670 (so it straddles two blocks) and reads it back 10 times.

**Expected output:** Prints 123450  (10 x 12345).

**Why the numbers look the way they do:** 16670 is 14 bytes into a 16-byte block and 30 bytes into a 32-byte block, so the word crosses a block boundary in both configurations. The first access is a two-block miss (1 + 8 + 2x7 = 23 cycles); later accesses are 1-cycle hits.

| No cache | DM 64x16 | FA 64x16 | DM 32x32 | FA 32x32 |
|---:|---:|---:|---:|---:|
| 558 | 211 | 211 | 209 | 209 |

### T11_array_copy.asm

**What it does:** Copies a 16-integer array (1..16) from the data segment to address 16640 with ILDR/ISTR, then sums the copy.

**Expected output:** Prints 136.

**Why the numbers look the way they do:** Interleaved sequential reads and writes at two locations, then a sequential read pass over the copy. Shows spatial locality for both reads and writes.

| No cache | DM 64x16 | FA 64x16 | DM 32x32 | FA 32x32 |
|---:|---:|---:|---:|---:|
| 2224 | 668 | 668 | 614 | 614 |

### T12_correctness.asm

**What it does:** Stores 11, 22, ... 88 at eight addresses 1024 bytes apart, then reads them back in reverse order.

**Expected output:** Prints 396  (11+22+...+88).

**Why the numbers look the way they do:** All eight addresses collide in a direct mapped cache, forcing dirty evictions. The printed value must be identical for every configuration: this is the data-correctness test for write-back.

| No cache | DM 64x16 | FA 64x16 | DM 32x32 | FA 32x32 |
|---:|---:|---:|---:|---:|
| 998 | 638 | 428 | 806 | 476 |

## Observations across the suite

- **Instruction caching:** T02 and T03 involve no data memory. T03 drops from 2020 to 446 cycles (4.5x faster) just from fetching a resident loop body.
- **Spatial locality:** T04, T09 and T11 improve with any cache; the larger 32-byte block helps sequential access (T04: 326 vs 296) and even 16-byte-strided access, where pairs of accesses now share a block (T05: 1308 vs 1182).
- **Conflicts:** T06 and T08 show direct mapped conflict misses. The fully associative cache is 2.6x faster than direct mapped in T06 (876 vs 2304) and 3.8x faster in T08 (400 vs 1520).
- **Caches can lose:** in T08 the 64x16 direct mapped cache (1520 cycles) is slower than no cache (1426), because each store pays a dirty write-back and a reload.
- **LRU is not always best:** in T07 the fully associative cache (5061) is slower than the direct mapped cache (3563) because a cyclic sweep over capacity + 1 blocks is the worst case for LRU.
- **Correctness:** the printed values in T02-T12 are identical in every configuration; only the cycle counts change.

## How the expected values were produced

Each program was run through an independent reference model (a flat-memory emulator with a tag-only timing model of each cache) and through the actual C++ emulator with its cache classes, at every configuration. The program outputs and all 60 cycle counts agree. Two small cases were also checked by hand: T03 without a cache is 202 fetches x 10 = 2020, and T04 without a cache is 85 fetches x 10 + 16 data reads x 8 = 978.