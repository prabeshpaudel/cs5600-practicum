#include "cache.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

CacheSlot cache[CACHE_SIZE];
int fifo_index = 0; // Index for FIFO replacement
int access_counter = 0; // Global access counter for LRU

// Checks if a message is in cache, returns index or -1
int check_cache(int id) {
    for (int i = 0; i < CACHE_SIZE; i++) {
        if (cache[i].valid && cache[i].id == id) {
            return i;
        }
    }
    return -1;
}

// Adds message to cache using FIFO strategy
void add_to_cache(Message* msg) {
    for (int i = 0; i < CACHE_SIZE; i++) {
        if (!cache[i].valid) {
            cache[i].valid = 1;
            cache[i].id = msg->id;
            cache[i].msg = *msg;
            printf("Message %d added to cache using FIFO at index %d.\n", msg->id, i);
            return;
        }
    }

    cache[fifo_index].id = msg->id;
    cache[fifo_index].msg = *msg;
    cache[fifo_index].valid = 1;
    cache[fifo_index].last_access = access_counter;
    printf("Message %d replaced cache slot using FIFO at index %d.\n", msg->id, fifo_index);

    fifo_index = (fifo_index + 1) % CACHE_SIZE;
}

// Adds message to cache using random replacement strategy
void add_to_cache_random(Message* msg) {
    for (int i = 0; i < CACHE_SIZE; i++) {
        if (!cache[i].valid) {
            cache[i].valid = 1;
            cache[i].id = msg->id;
            cache[i].msg = *msg;
            return;
        }
    }

    int victim = rand() % CACHE_SIZE;
    cache[victim].id = msg->id;
    cache[victim].msg = *msg;
    cache[victim].valid = 1;
    cache[victim].last_access = access_counter;
    printf("Message %d added to cache using Random Replacement at index %d.\n", msg->id, victim);
}

// Adds message to cache using LRU replacement strategy
void add_to_cache_lru(Message* msg) {
    int index = -1;
    int oldest_access = 100000;
    access_counter++;

    for (int i = 0; i < CACHE_SIZE; i++) {
        if (!cache[i].valid) {
            index = i;
            break;
        }
        if (cache[i].last_access < oldest_access) {
            oldest_access = cache[i].last_access;
            index = i;
        }
    }

    if (index == -1) {
        fprintf(stderr, "LRU Error: No valid index found.\n");
        exit(EXIT_FAILURE);
    }

    cache[index].valid = 1;
    cache[index].id = msg->id;
    cache[index].msg = *msg;
    cache[index].last_access = access_counter;
    printf("Message %d added to cache using LRU Replacement at index %d.\n", msg->id, index);
}

// Resets the cache
void reset_cache() {
    for (int i = 0; i < CACHE_SIZE; i++) {
        cache[i].valid = 0;
    }
    fifo_index = 0;
}