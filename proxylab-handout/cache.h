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
    int using_cache_num;
    cache_block cache_t[MAX_CACHE_SET_NUM];
} cache;



void init_cache();
int check_cache_block_exist(rio_t *rio_p,char *url);
void add_cache_block(char *url, char* content, int content_size);
