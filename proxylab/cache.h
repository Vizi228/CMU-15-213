/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400


struct cache_block
{
    char *url;
    char *data;
    unsigned long data_size;
    struct cache_block *next;
};

typedef struct 
{
    unsigned long size;
    struct cache_block *head;
} web_cache;

void init_cache();
struct cache_block *read_cache(char *url);
void write_cache(char *url, char *buf, unsigned long size);