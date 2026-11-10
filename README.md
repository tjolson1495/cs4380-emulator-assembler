# CS 4380: 4380 emulator and assembler

```
assembler/
  asm4380.py                 the assembler
  tests/                     pytest tests (run `pytest` from assembler/)
    conftest.py              makes `import asm4380` work from anywhere
    test_cli.py              command line behavior: usage, exit codes, output file
    test_cli_spec.py         assembly rules, using tests/fixtures/spec
    test_branches.py         JMR and branch instructions, using fixtures/asm + fixtures/expected
    test_examples.py         example / minimal / default .int programs
    test_mark2.py            Mark 2 opcodes and operand forms
    fixtures/
      spec/                  inputs and expected .bin files side by side
      asm/                   .asm inputs
      expected/              hand-checked .bin files to compare against
emulator/
  include/                   emu4380.h, cache.h
  src/                       main.cpp, emu4380.cpp
  tests/
    tests.cpp                GTest unit and system tests
    test_mark2.cpp           GTest tests for the Mark 2 instructions (not in the build yet)
    cache/                   cache validation programs T01..T12, results.md, run_cache_tests.sh
  Makefile, CMakeLists.txt
```

## Build

`make` in `emulator/` builds `emu4380` and `runTests` (GTest). `make clean` removes them.

## Run the tests

- Assembler: `pytest` in `assembler/`.
- Emulator unit tests: `./runTests` in `emulator/`.
- Cache programs: `emulator/tests/cache/run_cache_tests.sh`. It assembles each program in a
  temporary folder, runs it with cache types 0, 1 and 2, and compares the cycle counts with
  the expected numbers in `results.md`. Pass `32x32` as the first argument for an emulator
  built with 32 lines of 32-byte blocks.

## Generated files

Compiled programs, `.o` files and `.bin` files are not tracked (see `.gitignore`); the only
tracked `.bin` files are the expected outputs in `assembler/tests/fixtures/expected` and
`assembler/tests/fixtures/spec`.
