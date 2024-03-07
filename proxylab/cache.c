#include "cache.h"
#include "csapp.h"

web_cache *cache;
sem_t mutex, w;
int readcnt;

void move_end(struct cache_block *block);

void init_cache () {
    cache = Malloc(sizeof(web_cache));
    cache->size = 0;
    cache->head = Malloc(sizeof(struct cache_block));
    memset(cache->head, 0, sizeof(struct cache_block));

    Sem_init(&mutex, 0, 1);
    Sem_init(&w, 0, 1);
    readcnt = 0;
}

struct cache_block *read_cache(char *url) {
    struct cache_block *curr;
    P(&mutex);
    readcnt++;
    if (readcnt == 1) {
        P(&w);
    }
    V(&mutex);

    for(curr = cache->head->next; curr != NULL; curr = curr->next) {
        int url_len = strlen(url);
        int curr_url_len = strlen(curr->url);
        if (curr_url_len != url_len) {
            continue;
        }

        if (!strncmp(url, curr->url, strlen(url))) {
            printf("Returned from cache(%s)\n", url);
            break;
        }
    }

    P(&mutex);
    readcnt--;
    if (readcnt == 0) {
        V(&w);
    }
    V(&mutex);

    return curr;
}

void write_cache(char *url, char *buf, unsigned long size) {
    if (size > MAX_OBJECT_SIZE) {
        return;
    }

    P(&w);
    while((cache->size + size) > MAX_CACHE_SIZE) {
        struct cache_block *head = cache->head;
        cache->size -= head->data_size;
        cache->head = cache->head->next;
        free(head->data);
        free(head->url);
        free(head);
    }
    struct cache_block *new_block = Malloc(sizeof(struct cache_block));
    new_block->data_size = size;
    new_block->data = Malloc(size);
    memcpy(new_block->data, buf, size);
    new_block->url = Malloc(strlen(url));
    strcpy(new_block->url, url);

    // insert into start of cache
    new_block->next = cache->head->next;
    cache->head->next = new_block;

    // move to end, cause this is new block
    move_end(new_block);
    V(&w);
}

void move_end(struct cache_block *block) {
    struct cache_block *prev = NULL;
    struct cache_block *curr = cache->head;
    while(block != curr) {
        prev = curr;
        curr = curr->next;
    }

    if (prev != NULL) {
        prev->next = curr->next;
        block->next = NULL;
    }

    curr = cache->head;
    while(curr->next != NULL) {
        curr = curr->next;
    }
    curr->next = block;
}