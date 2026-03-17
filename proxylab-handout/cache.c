#include "cache.h"

static cache cache_t;
static sem_t mutex, w;
static int readcnt, operation_counter;





void init_cache() {
    operation_counter = 0;
    readcnt = 0;
    cache_t.using_cache_num = 0;
    sem_init(&mutex, 0, 1); // pshared 0 means multi thread share, not 0 means multi process share
    sem_init(&w, 0, 1);
}


int check_cache_block_exist(rio_t *rio_p,char *url) {
    P(&mutex);
    readcnt++; //because the mutex lock the readcnt can express the order of reader exactly
    if (readcnt == 1)
    {
        P(&w); // let first reader get the lock of writer to implement the reader first
    }
    V(&mutex); //unlock for the next part needn't the lock

    int hit_flag = 0;
    for (int i = 0; i < MAX_CACHE_NUM;i++) {
        // hit cache
        if (!strcmp(cache_t.cache_blocks[i].url, url)) {
            // 
            P(&mutex);
            cache_t.cache_blocks[i].operation_counter = operation_counter++;
            V(&mutex);
            // ·¢ËÍ»º´æÄÚÈÝ
            rio_writen(rio_p->rio_fd, cache_t.cache_blocks[i].content, cache_t.cache_blocks[i].content_size);
            hit_flag = 1;
            break;
        }
    }


    P(&mutex);

    readcnt--;

    if (readcnt == 0)
    {
        V(&w); // the last reader free the writer lock
    }
    
    V(&mutex);
    if (hit_flag) {
        return 1;
    }
    return 0;

}







void add_cache_block(char *url, char* content, int content_size) {
    P(&w);

    if (cache_t.using_cache_num == (MAX_CACHE_NUM - 1)) {
        // find the oldest cache block, it means the block which has the lowest operation counter
        int oldest_index;
        int oldest_operation_counter = operation_counter;
        for (int i = 0;i < MAX_CACHE_NUM;i++) {
            if (cache_t.cache_blocks[i].operation_counter < oldest_operation_counter) {
                oldest_operation_counter = cache_t.cache_blocks[i].operation_counter;
                oldest_index = i;
            }
        }
        // replace the oldest cache block
        strcpy(cache_t.cache_blocks[oldest_index].url, url);
        memcpy(cache_t.cache_blocks[oldest_index].content, content, content_size);
        cache_t.cache_blocks[oldest_index].content_size = content_size;
        // update operation counter, use lock because it is a static variable all threads can access it.
        P(&mutex);
        cache_t.cache_blocks[oldest_index].operation_counter = operation_counter++;
        V(&mutex);
    }
    // cache isn't full
    else {
        // add cache block
        strcpy(cache_t.cache_blocks[cache_t.using_cache_num].url, url);
        memcpy(cache_t.cache_blocks[cache_t.using_cache_num].content, content, content_size);
        cache_t.cache_blocks[cache_t.using_cache_num].content_size = content_size;
        // update operation counter, use lock because it is a static variable all threads can access it.
        P(&mutex);
        // the read or the write operation all add the operation counter, it means the counter larger the block newer
        cache_t.cache_blocks[cache_t.using_cache_num].operation_counter = operation_counter++;
        V(&mutex);
        cache_t.using_cache_num++;
    }
    // ½âËø
    V(&w);
    return 0;
}