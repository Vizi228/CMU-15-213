#include "cachelab.h"
#include "stdlib.h"
#include <stdio.h>
#include "getopt.h"
#include "unistd.h"

typedef unsigned count;
typedef char flag;

unsigned m = sizeof(void *) * 8;
unsigned s, E, S, b;
size_t t;
flag v;
count misses, hits, evictions;

struct Line
{
    char valid;
    unsigned tag;
    struct Line *next;
};

struct Set
{
    struct Line *lineHead;
};

struct Cache
{
    struct Set *sets;
};

struct Cache *initCache();
void freeCache(struct Cache *cache);
void requestToCache(struct Cache *cache, unsigned address);
void rearrangeQueue(struct Set *set, struct Line *currentLine, struct Line *prevLine);
size_t getSetSize(struct Set *set);

int main(int argc, char *argv[])
{
    char opt;
    char *trace;

    while ((opt = getopt(argc, argv, "s:E:b:t:v")) != -1)
    {
        switch (opt)
        {
        case 's':
            s = atoi(optarg);
            break;
        case 'E':
            E = atoi(optarg);
            break;
        case 'b':
            b = atoi(optarg);
            break;
        case 't':
            trace = optarg;
            break;
        case 'v':
            v = 1;
            break;
        default:
            break;
        }
    }

    // determine size of tag
    t = m - s - b;
    S = (0x1 << s);

    FILE *fptr;
    fptr = fopen(trace, "r");

    if (fptr == NULL)
    {
        printf("File doesn't exist!");
        exit(1);
    }

    struct Cache *cache = initCache();

    char operation[1];
    unsigned address;
    unsigned size;

    while ((fscanf(fptr, " %c %x,%d", operation, &address, &size)) != EOF)
    {
        if (v) printf("%c %x,%d", *operation, address, size);
        int prevMisses = misses, prevHits = hits, prevEvictions = evictions;
        switch (*operation)
        {
        case 'L':
        case 'S':
            requestToCache(cache, address);
            break;
        case 'M':
            requestToCache(cache, address);
            requestToCache(cache, address);
            break;
        default:
            break;
        }
        if (v) {
            if (misses - prevMisses) printf(" miss");
            if (hits - prevHits) printf(" hit");
            if (evictions - prevEvictions) printf(" eviction");
            printf("\n");
        }
    }

    printSummary(hits, misses, evictions);
    freeCache(cache);
    return 0;
}

struct Cache *initCache()
{
    struct Cache *cache = malloc(sizeof(struct Cache));
    cache->sets = malloc(sizeof(struct Set) * S);

    for (unsigned i = 0; i < S; i++)
    {
        struct Set *set = &cache->sets[i];
        set->lineHead = malloc(sizeof(struct Line));
        set->lineHead->tag = 0;
        set->lineHead->valid = 0;
        struct Line *currentLine = set->lineHead;

        for (unsigned j = 1; j < E; j++)
        {
            currentLine->next = malloc(sizeof(struct Line));
            currentLine = currentLine->next;
            currentLine->tag = j;
            currentLine->valid = 0;
        }
        currentLine->next = NULL;
    }

    return cache;
}

void freeCache(struct Cache *cache)
{
    for (unsigned i = 0; i < S; i++)
    {
        struct Set *set = &cache->sets[i];
        struct Line *currentLine = set->lineHead;
        while (currentLine != NULL)
        {
            struct Line *tmp = currentLine;
            currentLine = currentLine->next;
            free(tmp);
        }
    }
    free(cache->sets);
    free(cache);
}

void requestToCache(struct Cache *cache, unsigned address)
{
    unsigned setIndex = (address << t) >> (t + b);
    unsigned tag = address >> (s + b);
    if (setIndex >= S || setIndex < 0)
    {
        misses++;
        return;
    }

    struct Set *set = &cache->sets[setIndex];
    struct Line *currentLine = set->lineHead;

    if (E == 1)
    {
        if (currentLine != NULL && currentLine->valid)
        {
            if (currentLine->tag == tag)
            {
                hits++;
            }
            else
            {
                evictions++;
                misses++;
                currentLine->tag = tag;
            }
        }
        else
        {
            misses++;
            currentLine->tag = tag;
            currentLine->valid = 1;
        }
        return;
    }

    struct Line *prevLine = NULL;
    while (currentLine != NULL && (currentLine->tag != tag || currentLine->valid == 0))
    {
        prevLine = currentLine;
        currentLine = currentLine->next;
    }

    if (currentLine == NULL)
    {
        if (getSetSize(set) == E)
        {
            evictions++;
        }
        misses++;
        struct Line *newLine = malloc(sizeof(struct Line));
        newLine->tag = tag;
        newLine->valid = 1;
        rearrangeQueue(set, newLine, NULL);
    }
    else
    {
        hits++;
        rearrangeQueue(set, currentLine, prevLine);
    }
}

size_t getSetSize(struct Set *set)
{
    size_t setSize = 0;
    struct Line *currentLine = set->lineHead;
    while (currentLine != NULL)
    {
        setSize += currentLine->valid;
        currentLine = currentLine->next;
    }
    return setSize;
}

void rearrangeQueue(struct Set *set, struct Line *currentLine, struct Line *prevLine)
{
    if (prevLine == NULL)
    {
        struct Line *tmp = set->lineHead;
        set->lineHead = set->lineHead->next;
        prevLine = set->lineHead;
        if (currentLine != tmp)
        {
            free(tmp);
        }
    } else {
        prevLine->next = currentLine->next;
    }

    while (prevLine->next)
    {
        prevLine = prevLine->next;
    }
    prevLine->next = currentLine;
    currentLine->next = NULL;
}