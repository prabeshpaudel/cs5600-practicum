#ifndef CACHE_H
#define CACHE_H

#include <time.h>
#include "message.h"

#define CACHE_SIZE 16  // number of cache slots

// Structure representing a single cache slot
typedef struct {
    int valid;          // indicates if cache slot is in use
    int id;             // ID of the cached message
    Message msg;        // cached message data
    int last_access;    // last access timestamp (for LRU)
} CacheSlot;

CacheSlot cache[CACHE_SIZE];  // cache array
int fifo_index;               // index for FIFO replacement strategy (if used)

// checks if message is in cache, returns index or -1 if not found
int check_cache(int id);

// adds message to cache using default (FIFO) method
void add_to_cache(Message* msg);

// adds message using random replacement method
void add_to_cache_random(Message* msg);

// adds message using Least Recently Used (LRU) method
void add_to_cache_lru(Message* msg);

// resets cache slots to initial state
void reset_cache();

#endif // CACHE_H