#ifndef CACHE_H
#define CACHE_H

#include <vector>

// Compile-time cache configuration (Req7, Req8, Req12)
#define CACHE_LINES      64
#define CACHE_BLOCK_SIZE 16

class Line {
    public:
        bool valid;
        bool dirty;
        unsigned int tag;                  // full block number of the block held
        unsigned int last_used;            // LRU stamp
        std::vector<unsigned char> data;   // the block's bytes

        Line(unsigned int block_size);
};

class Cache {

    protected:
        unsigned int lines_count;
        unsigned int block_size;
        unsigned int words_per_block;
        unsigned int use_counter;          // increases on every access, feeds last_used
        std::vector<Line> lines;

        // Lines in which this block is allowed to live (defined per cache type)
        virtual std::vector<Line*> find_candidates(unsigned int block) = 0;

        // Line to replace when the block is not present (defined per cache type)
        virtual Line* choose_victim(unsigned int block) = 0;

        // An invalid candidate if there is one, otherwise the least recently used
        Line* find_lru(std::vector<Line*>& candidates);

        // Copies the line's bytes back into memory. Charges no cycles.
        void write_back(Line& line);

        // Copies a block from memory into the line. Charges no cycles.
        void load_block(Line& line, unsigned int block);

        // Records that the line was just used (for LRU)
        void touch(Line& line);

        // 8 cycles for the first word, 2 for each additional word
        unsigned int burst_cycles(unsigned int words);

        // Makes sure the block is in the cache and returns its line.
        // Counts blocks read from memory and blocks written back.
        Line* get_block(unsigned int block, unsigned int& loads, unsigned int& writebacks);

    public:
        Cache(unsigned int ln, unsigned int bs);
        virtual ~Cache();

        // Returns the size bytes (1 or 4) at address, assembled little-endian
        unsigned int read(unsigned int address, unsigned int size);

        // Stores the low size bytes (1 or 4) of value at address, little-endian
        void write(unsigned int address, unsigned int size, unsigned int value);
};

class DirectMappedCache : public Cache {

    protected:
        std::vector<Line*> find_candidates(unsigned int block) override;
        Line* choose_victim(unsigned int block) override;

    public:
        DirectMappedCache(unsigned int ln, unsigned int bs);
};

class FullyAssociativeCache : public Cache {

    protected:
        std::vector<Line*> find_candidates(unsigned int block) override;
        Line* choose_victim(unsigned int block) override;

    public:
        FullyAssociativeCache(unsigned int ln, unsigned int bs);
};

class TwoWaySetCache : public Cache {

    protected:
        unsigned int sets_count;

        std::vector<Line*> find_candidates(unsigned int block) override;
        Line* choose_victim(unsigned int block) override;

    public:
        TwoWaySetCache(unsigned int ln, unsigned int bs);
};

#endif