#include "csapp.h"


#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
#define MAX_CACHE_SET_NUM 10
#define MAX_CACHE_SET_BLOCK_NUM 5


typedef struct
{
    char url[MAXLINE];
    char content[MAX_OBJECT_SIZE];
    int content_size;
    int timestamp;
} cache_block;



typedef struct 
{
    
} cache_set ;


typedef struct 
{
    /* data */
} cache_t;
