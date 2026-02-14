#include "cachelab.h"
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>

void printUsage(char* argv[]);



int verbose = 0; // Global variable to track verbose flag


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
    int opt;
    while ((opt = getopt(argc, argv, "hvs:E:b:t:")) != -1) // Parse command-line arguments
    {
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
            break;
        case 'E':
            // Lines per set
            break;
        case 'b':
            // Block offset bits
            break;
        case 't':
            // Trace file
            trace_fp = fopen(optarg, "r");
            if (trace_fp == NULL) {
                printf("Error opening trace file %s\n", optarg);
                exit(1);
            }
            break;
        
        default:
            break;
        }
    } 
    printSummary(0, 0, 0);
    return 0;
}

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
