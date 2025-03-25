#include "cache.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

CacheSlot cache[CACHE_SIZE];
int fifo_index = 0; // Tracks the oldest message
int access_counter = 0;



int check_cache(int id) {
    for (int i = 0; i < CACHE_SIZE; i++) {
        if (cache[i].valid && cache[i].id == id) {
            //return &cache[i].msg;
            return i;
        }
    }
    return -1;
}

void add_to_cache(Message* msg) {
    // Check for an empty slot first
    for (int i = 0; i < CACHE_SIZE; i++) {
        if (!cache[i].valid) {
            cache[i].valid = 1;
            cache[i].id = msg->id;
            cache[i].msg = *msg;
            printf("Message %d added to cache using FIFO at index %d.\n", msg->id, i);
            return;
        }
    }

    // Perform FIFO replacement if cache is full
    cache[fifo_index].id = msg->id;
    cache[fifo_index].msg = *msg;
    cache[fifo_index].valid = 1;
    cache[fifo_index].last_access = access_counter; // Track the time
    printf("Message %d replaced cache slot using FIFO at index %d.\n", msg->id, fifo_index);

    // Update the FIFO index to the next slot in a circular manner
    fifo_index = (fifo_index + 1) % CACHE_SIZE;

}


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
    cache[victim].last_access = access_counter; // Track the time
    printf("Message %d added to cache using Random Replacement at index %d.\n", msg->id, victim);


}

void add_to_cache_lru(Message* msg) {
    int index = -1;
    int oldest_access = 100000;
    access_counter++;


    // Find an empty slot or the least recently used one
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
   // printf("Message: %d OLDEST ACCESS: %d \n", msg->id, oldest_access);


    // Replace and update the cache
    cache[index].valid = 1;
    cache[index].id = msg->id;
    cache[index].msg = *msg;
    cache[index].last_access = access_counter;
    printf("Message %d added to cache using LRU Replacement at index %d.\n", msg->id, index);
}


void reset_cache() {
    for (int i = 0; i < CACHE_SIZE; i++) {
        cache[i].valid = 0;
    }
    fifo_index = 0;
}
