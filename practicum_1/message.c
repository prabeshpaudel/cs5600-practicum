#include "message.h"
#include "cache.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int cache_hits = 0;
int cache_misses = 0;
Message* create_msg(int id, const char* sender, const char* receiver, const char* content) {

    Message* msg = (Message*)malloc(sizeof(Message));

    if (msg == NULL) {
        printf("Error: Memory allocation failed\n");
        return NULL;
    }

    msg->id = id;
    msg->timestamp = time(NULL);
    strcpy(msg->sender, sender);
    strcpy(msg->receiver, receiver);
    strcpy(msg->content, content);
    msg->delivered = 0;

    return msg;
}

int store_msg_to_disk(Message* msg) {

    if (msg == NULL) {
        printf("Error: Message is NULL\n");
        return -1;
    }

    char filename[50];
    snprintf(filename, sizeof(filename), "messages/message_%d.txt", msg->id);

    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        printf("Error: File could not be opened\n");
        return -1;
    }

    fprintf(file, "ID: %d\n", msg->id);
    fprintf(file, "Timestamp: %ld\n", msg->timestamp);
    fprintf(file, "Sender: %s\n", msg->sender);
    fprintf(file, "Receiver: %s\n", msg->receiver);
    fprintf(file, "Content: %s\n", msg->content);
    fprintf(file, "Delivered: %d\n", msg->delivered);

    fclose(file);

    printf("Message %d stored successfully.\n", msg->id);
    return 0;
}


Message* retrieve_msg_from_disk(int id) {
    char filename[50];
    snprintf(filename, sizeof(filename), "messages/message_%d.txt", id);

    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        return NULL;
    }

    Message* msg = (Message*)malloc(sizeof(Message));
    if (msg == NULL) {
        fprintf(stderr, "Memory allocation failed!\n");
        fclose(file);
        return NULL;
    }

    fscanf(file, "ID: %d\n", &msg->id);
    fscanf(file, "Timestamp: %ld\n", &msg->timestamp);
    fscanf(file, "Sender: %[^\n]\n", msg->sender);
    fscanf(file, "Receiver: %[^\n]\n", msg->receiver);
    fscanf(file, "Content: %[^\n]\n", msg->content);
    fscanf(file, "Delivered: %d\n", &msg->delivered);

    fclose(file);

    return msg;
}
void handle_cache(Message* msg, const char* method) {
    if (strcmp(method, "Random") == 0) {
        add_to_cache_random(msg);
    } else if (strcmp(method, "LRU") == 0) {
        add_to_cache_lru(msg);
    } else {
        add_to_cache(msg);
    }
}



int store_msg(Message* msg, const char* method) {
    int result = store_msg_to_disk(msg);
    if (result == 0) {
        handle_cache(msg, method);
    }
return result;
}

Message* retrieve_msg(int id, const char* method) {
    int cached_index = check_cache(id);
    Message* cached = &cache[cached_index].msg;
    if (cached_index != -1) {
        cache_hits++;
        printf("[Cache HIT]\n");
        cache[cached_index].last_access = access_counter++;
        printf("Message %d reloaded from cache at index %d. with %d access count \n", cached->id, cached_index,cache[cached_index].last_access );
        return cached;
    }
    
    printf("[Cache MISS]\n");
    cache_misses++;
    Message* loaded = retrieve_msg_from_disk(id);
    if (loaded != NULL) {
        handle_cache(loaded, method);
    }

    return loaded;
}

void reset_hm () {
    cache_hits = 0;
    cache_misses = 0;
}
