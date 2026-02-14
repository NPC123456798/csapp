#include "cachelab.h"
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
typedef struct {
    int set_index; // number of set index bits
    int E; // number of lines per set
    int block_offset; // number of block offset bits
    int S; // number of sets
    // int B;
    cache_set_t* sets;
} cache;

typedef struct {
    int valid;
    uint64_t tag;
    int lru_counter; // For LRU eviction policy
} cache_line_t;

typedef struct {
    cache_line_t* lines;
} cache_set_t;

int main(int argc, char* argv[])
{
    FILE* trace_fp = NULL;
    printSummary(0, 0, 0);
    return 0;
}
