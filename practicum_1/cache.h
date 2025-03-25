#ifndef CACHE_H
#define CACHE_H

#include <time.h>
#include "message.h"

#define CACHE_SIZE 16


typedef struct {
    int valid;
    int id;
    Message msg;
    int last_access;
} CacheSlot;

CacheSlot cache[CACHE_SIZE];
int fifo_index;

int check_cache(int id);
void add_to_cache(Message* msg);
void add_to_cache_random(Message* msg);
void add_to_cache_lru(Message* msg);
void reset_cache();

#endif // CACHE_H
