#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "emu4380.h"

using namespace std;

#define DEFAULT_SIZE 131072

int main(int argc, char* argv[]) {

    if (argc < 2) {
        cout << "Usage: .build/emu4380 <.bin filename> [memory size]" << endl;
        return 1;
    }

    string filename = argv[1];
    unsigned int size;

    if (argc > 2) {
  
            try {
                unsigned long parsed = std::stoul(argv[2]);

                if (parsed == 0 || parsed > 4294967295UL) {
                    std::cerr << "Error: memory size must be a positive integer <= 4294967295\n";
                    return 1;
                }

                size = static_cast<unsigned int>(parsed);

            } catch (const std::invalid_argument&) {
                std::cerr << "Error: memory size must be a valid integer\n";
                return 1;
            } catch (const std::out_of_range&) {
                std::cerr << "Error: memory size is too large\n";
                return 1;
            } 

    } else {
        size = DEFAULT_SIZE;
    }

    init_mem(size);
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