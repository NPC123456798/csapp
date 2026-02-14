#include "cachelab.h"
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>

typedef struct cache_line{
    int valid;
    uint64_t tag;
    int lru_counter; // For LRU eviction policy
} cache_line_t;

typedef struct cache_sets{
    cache_line_t* lines;
} cache_set_t;


typedef struct cache{
    int set_index; // number of set index bits
    int E; // number of lines per set
    int block_offset; // number of block offset bits
    int S; // number of sets
    // int B;
    cache_set_t* sets;
} cache;



void free_cache(cache* c);
void printUsage(char* argv[]);
void counter_LRU(cache* c);
void handle_argument_error(char* argv[]);
void set_cache(cache* c);
void simulate_cache(cache* c, FILE* trace_fp);
void load_cache(cache* c, uint64_t address, int size);
void store_cache(cache* c, uint64_t address, int size);
void modify_cache(cache* c, uint64_t address, int size);


int verbose = 0; // Global variable to track verbose flag
int hits = 0;
int misses = 0;
int evictions = 0; // Global variable to track eviction count













int main(int argc, char* argv[])
{
    FILE* trace_fp = NULL;
    int opt;
    cache* c = malloc(sizeof(cache));
    while ((opt = getopt(argc, argv, "hvs:E:b:t:")) != -1) // Parse command-line arguments
    {
        int argument = 0;
        if (optarg != NULL) {
            argument = atoi(optarg); // Convert argument to integer
        }
        switch (opt)
        {
            case 'h':
                printUsage(argv);
                exit(0);
            case 'v':
                // Verbose flag
                verbose = 1;
                break;
            case 's':
                // Set index bits
                if (argument == 0)
                {
                    handle_argument_error(argv);
                }
                
                c->set_index = argument;
                c->S = 1 << c->set_index; // Calculate number of sets
                break;
            case 'E':
                // Lines per set
                if (argument == 0)
                {
                    handle_argument_error(argv);
                }
                c->E = argument;
                break;
            case 'b':
                // Block offset bits
                if (argument == 0)
                {
                    handle_argument_error(argv);
                }
                c->block_offset = argument;
                break;
            case 't':
                // Trace file
                trace_fp = fopen(optarg, "r");
                if (trace_fp == NULL) {
                    printf("%s: No such file or directory\n", optarg);
                    exit(1);
                }
                break;
            case '?':        
                printUsage(argv);
                exit(1);
            default:
                break;
        }
    } 
    set_cache(c); // Initialize cache structure
    // printSummary(hits, misses, evictions);
    free_cache(c);
    fclose(trace_fp);
    return 0;
}

/* print the help message */
void printUsage(char* argv[]) {
    printf("Usage: %s [-hv] -s <num> -E <num> -b <num> -t <file>\n", argv[0]);
    printf("Options:\n");
    printf("  -h         Print this help message.\n");
    printf("  -v         Optional verbose flag.\n");
    printf("  -s <num>     Number of set index bits.\n");
    printf("  -E <num>     Number of lines per set.\n");
    printf("  -b <num>     Number of block offset bits.\n");
    printf("  -t <file>    Trace file.\n");
    printf("\nExamples:\n");
    printf("linux> %s -s 4 -E 1 -b 4 -t traces/yi.trace\n", argv[0]);
    printf("linux> %s -v -s 8 -E 2 -b 4 -t traces/yi.trace\n", argv[0]);
}

/* Increase LRU counter for all valid lines in cache */
void counter_LRU(cache* c) {
    for (int i = 0; i < c->S; i++) {
        for (int j = 0; j < c->E; j++) {
            if (c->sets[i].lines[j].valid) {
                c->sets[i].lines[j].lru_counter++;
            }
        }
    }
}

void handle_argument_error(char* argv[]) {
    printf("Error: Missing required command line argument\n");
    printUsage(argv);
    exit(1);
}

void free_cache(cache* c) {
    for (int i = 0; i < c->S; i++) {
        free(c->sets[i].lines);
    }
    free(c->sets);
    free(c);
}


void set_cache(cache* c) {
    c->sets = malloc(c->S * sizeof(cache_set_t)); // Allocate memory for sets
    for (int i = 0; i < c->S; i++) {
        c->sets[i].lines = malloc(c->E * sizeof(cache_line_t)); // Allocate memory for lines in each set
        for (int j = 0; j < c->E; j++) {
            c->sets[i].lines[j].valid = 0;
            c->sets[i].lines[j].tag = 0;
            c->sets[i].lines[j].lru_counter = 0;
        }
    }
}


void simulate_cache(cache* c, FILE* trace_fp) {
    char operation;
    uint64_t address;
    int size;
    while (fscanf(trace_fp, " %c %lx,%d", &operation, &address, &size) == 3) {
        // Process the trace line and update hits, misses, evictions
        // Implement cache simulation logic here
        switch (operation)
        {
            // Simulate cache access for load, store, and modify operations
            case 'L':
                load_cache(c, address, size);
                break;
            case 'S':
                store_cache(c, address, size);
                break;
            case 'M':
                modify_cache(c, address, size);
                break;
            default:
                break;
        }
        
    }
}



void load_cache(cache* c, uint64_t address, int size) {
    // Implement cache load logic here
    if (verbose == 1)
    {
        printf("L %lx,%d ", address, size);
    }
    counter_LRU(c); // Increment LRU counters for all valid lines before accessing the cache
    // ((1 << c->set_index) - 1) is used to create a mask for extracting the set index bits from the address. 
    //  like if set_index is 4, then (1 << 4) - 1 = 15 (0b1111), which allows us to extract the last 4 bits of the address for the set index.
    uint64_t set_index = (address >> c->block_offset) & ((1 << c->set_index) - 1);
    uint64_t tag = address >> (c->block_offset + c->set_index);
    cache_set_t* set = &c->sets[set_index]; // use & to get the address of the set to modify it directly instead of a copy
    cache_line_t* lines = set->lines;
    for (size_t i = 0; i < c->E; i++)
    {
        if (lines[i].valid && lines[i].tag == tag)
        {
            lines[i].lru_counter = 0;
            if (verbose == 1)
            {
                printf(" hit");
            }
            return;
        }
    }
    // If not found, we need to evict a line and insert the new one
    int evict_index = 0;
    for (size_t i = 0; i < c->E; i++)
    {
        if (lines[i].valid == 0)
        {
            evict_index = i;
            break;
        }
        else if (lines[i].lru_counter > lines[evict_index].lru_counter)
        {
            evict_index = i;
        }
    }
    // Evict the line with highest LRU counter
    lines[evict_index].valid = 1;
    lines[evict_index].tag = tag;
    lines[evict_index].lru_counter = 0;
    if (verbose == 1)
    {
        printf(" miss");
    }
    

    
}


void store_cache(cache* c, uint64_t address, int size) {
    // Implement cache store logic here
}

void modify_cache(cache* c, uint64_t address, int size) {
    // Implement cache modify logic here
}