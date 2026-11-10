#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "emu4380.h"

using namespace std;

#define DEFAULT_SIZE 131072

void print_usage() {
    cout << "Usage: .build/emu4380 <.bin filename> [-m memory size] [-c cache type]" << endl;
}

bool validate_memory(const string& arg, unsigned int& size) {

    // Digits only: rejects signs, spaces, trailing text, and an empty string
    if (arg.empty() || arg.find_first_not_of("0123456789") != string::npos) {
        cerr << "Error: memory size must be a positive integer\n";
        return false;
    }

    try {
        unsigned long long parsed = stoull(arg);

        if (parsed == 0 || parsed > 4294967295ULL) {
            cerr << "Error: memory size must be a positive integer <= 4294967295\n";
            return false;
        }

        size = static_cast<unsigned int>(parsed);
        return true;

    } catch (const out_of_range&) {
        cerr << "Error: memory size is too large\n";
        return false;
    }
}

// Valid cache types: 0 = none, 1 = direct mapped, 2 = fully associative, 3 = 2-way set associative
bool validate_cache(const string& arg, unsigned int& cache_type) {

    if (arg.size() != 1 || arg[0] < '0' || arg[0] > '3') {
        cout << "Invalid cache configuration. Aborting.\n";
        return false;
    }

    cache_type = static_cast<unsigned int>(arg[0] - '0');
    return true;
}

int main(int argc, char* argv[]) {

    if (argc < 2) {
        print_usage();
        return 1;
    }

    string filename = argv[1];
    unsigned int size = DEFAULT_SIZE;
    unsigned int cache_type = 0;            // default: no cache

    for (int i = 2; i < argc; i++) {
        string flag = argv[i];

        if (flag == "-m") {

            if (i + 1 >= argc) {            // flag with no value
                print_usage();
                return 1;
            }
            if (!validate_memory(argv[++i], size)) return 2;

        } else if (flag == "-c") {

            if (i + 1 >= argc) {            // a missing value is an invalid cache configuration
                cout << "Invalid cache configuration. Aborting.\n";
                return 2;
            }
            if (!validate_cache(argv[++i], cache_type)) return 2;

        } else {                            // unrecognized argument
            print_usage();
            return 1;
        }
    }

    init_mem(size);
    init_cache(cache_type);

    ifstream MyReadFile(filename, ios::binary);

    if (!MyReadFile.is_open()) {
        cout << "Error: could not open file " << filename << endl;
        return 1;
    }

    MyReadFile.seekg(0, ios::end);
    streamsize fileSize = MyReadFile.tellg();
    MyReadFile.seekg(0, ios::beg);

    if (fileSize < 0 || static_cast<unsigned long>(fileSize) > MEM_SIZE) {
        cout << "INSUFFICIENT MEMORY SPACE" << endl;
        return 2;
    }

    MyReadFile.read(reinterpret_cast<char*>(prog_mem), fileSize);
    MyReadFile.close();

    reg_file[PC] = *reinterpret_cast<unsigned int*>(prog_mem);
    reg_file[SL] = static_cast<unsigned int>(fileSize);
    reg_file[SB] = MEM_SIZE;
    reg_file[SP] = MEM_SIZE;
    reg_file[FP] = 0;
    reg_file[HP] = reg_file[SL];

    while (true) {
        if (!fetch())   { invalid_instruction(reg_file[PC]);     break; }
        if (!decode())  { invalid_instruction(reg_file[PC] - 8); break; }
        if (!execute()) { invalid_instruction(reg_file[PC] - 8); break; }
    }

    return 0;
}