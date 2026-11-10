#include "cache.h"
#include "emu4380.h"        // MEM_SIZE, prog_mem, mem_cycle_cntr

// ---------------------------------------------------------------------
// Line
// ---------------------------------------------------------------------

Line::Line(unsigned int block_size)
    : valid(false), dirty(false), tag(0), last_used(0), data(block_size, 0) {}

// ---------------------------------------------------------------------
// Cache (shared behavior)
// ---------------------------------------------------------------------

Cache::Cache(unsigned int ln, unsigned int bs)
    : lines_count(ln),
      block_size(bs),
      words_per_block(bs / 4),
      use_counter(0),
      lines(ln, Line(bs)) {}

Cache::~Cache() {}

Line* Cache::find_lru(std::vector<Line*>& candidates) {
    Line* best = candidates[0];

    for (Line* c : candidates) {
        if (!c->valid) return c;
        if (c->last_used < best->last_used) best = c;
    }

    return best;
}

void Cache::write_back(Line& line) {
    unsigned int address = line.tag * block_size;

    for (unsigned int i = 0; i < block_size; i++) {
        if (address + i < MEM_SIZE) {
            prog_mem[address + i] = line.data[i];
        }
    }
}

void Cache::load_block(Line& line, unsigned int block) {
    unsigned int address = block * block_size;

    for (unsigned int i = 0; i < block_size; i++) {
        if (address + i < MEM_SIZE) {
            line.data[i] = prog_mem[address + i];
        } else {
            line.data[i] = 0;   // block hangs past the end of memory
        }
    }

    line.tag = block;
    line.valid = true;
    line.dirty = false;
}

void Cache::touch(Line& line) {
    use_counter++;
    line.last_used = use_counter;
}

unsigned int Cache::burst_cycles(unsigned int words) {
    if (words == 0) return 0;
    return 8 + 2 * (words - 1);
}

Line* Cache::get_block(unsigned int block, unsigned int& loads, unsigned int& writebacks) {

    std::vector<Line*> candidates = find_candidates(block);

    for (Line* c : candidates) {
        if (c->valid && c->tag == block) {
            touch(*c);
            return c;
        }
    }

    Line* victim = choose_victim(block);

    if (victim->valid && victim->dirty) {
        write_back(*victim);
        writebacks++;
    }

    load_block(*victim, block);
    loads++;

    touch(*victim);
    return victim;
}

unsigned int Cache::read(unsigned int address, unsigned int size) {

    unsigned int result = 0;
    unsigned int loads = 0;
    unsigned int writebacks = 0;

    for (unsigned int i = 0; i < size; i++) {
        unsigned int a = address + i;
        Line* line = get_block(a / block_size, loads, writebacks);

        result |= static_cast<unsigned int>(line->data[a % block_size]) << (8 * i);
    }

    mem_cycle_cntr += 1
                    + burst_cycles(loads * words_per_block)
                    + burst_cycles(writebacks * words_per_block);

    return result;
}

void Cache::write(unsigned int address, unsigned int size, unsigned int value) {

    unsigned int loads = 0;
    unsigned int writebacks = 0;

    for (unsigned int i = 0; i < size; i++) {
        unsigned int a = address + i;
        Line* line = get_block(a / block_size, loads, writebacks);

        line->data[a % block_size] = (value >> (8 * i)) & 0xFF;
        line->dirty = true;
    }

    mem_cycle_cntr += 1
                    + burst_cycles(loads * words_per_block)
                    + burst_cycles(writebacks * words_per_block);
}

// ---------------------------------------------------------------------
// Direct mapped
// ---------------------------------------------------------------------

DirectMappedCache::DirectMappedCache(unsigned int ln, unsigned int bs)
    : Cache(ln, bs) {}

std::vector<Line*> DirectMappedCache::find_candidates(unsigned int block) {
    return { &lines[block % lines_count] };
}

Line* DirectMappedCache::choose_victim(unsigned int block) {
    return &lines[block % lines_count];
}

// ---------------------------------------------------------------------
// Fully associative
// ---------------------------------------------------------------------

FullyAssociativeCache::FullyAssociativeCache(unsigned int ln, unsigned int bs)
    : Cache(ln, bs) {}

// Any line may hold any block, so the block number is not needed here
std::vector<Line*> FullyAssociativeCache::find_candidates(unsigned int) {
    std::vector<Line*> all;

    for (Line& l : lines) {
        all.push_back(&l);
    }

    return all;
}

Line* FullyAssociativeCache::choose_victim(unsigned int block) {
    std::vector<Line*> all = find_candidates(block);
    return find_lru(all);
}

// ---------------------------------------------------------------------
// 2-way set associative (extra credit)
// ---------------------------------------------------------------------

TwoWaySetCache::TwoWaySetCache(unsigned int ln, unsigned int bs)
    : Cache(ln, bs), sets_count(ln / 2) {}

std::vector<Line*> TwoWaySetCache::find_candidates(unsigned int block) {
    unsigned int set = block % sets_count;
    return { &lines[2 * set], &lines[2 * set + 1] };
}

Line* TwoWaySetCache::choose_victim(unsigned int block) {
    std::vector<Line*> both = find_candidates(block);
    return find_lru(both);
}