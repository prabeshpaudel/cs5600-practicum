#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define CACHE_SIZE 16

typedef struct {
    int id;
    time_t timestamp;
    char sender[50];
    char receiver[50];
    char content[256];
    int delivered;
} Message;

typedef struct {
    int valid;
    int id;
    Message msg;
    time_t last_access;
} CacheSlot;

CacheSlot cache[CACHE_SIZE];
int fifo_index = 0; // Tracks the oldest message


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

Message* check_cache(int id) {
    for (int i = 0; i < CACHE_SIZE; i++) {
        if (cache[i].valid && cache[i].id == id) {
            return &cache[i].msg;
            printf("%s", &cache[i].msg);

        }
    }
    return NULL;
}


void add_to_cache(Message* msg) {
    // Check for an empty slot first
    for (int i = 0; i < CACHE_SIZE; i++) {
        if (!cache[i].valid) {
            cache[i].valid = 1;
            cache[i].id = msg->id;
            cache[i].msg = *msg;
            return;
        }
    }

    // Perform FIFO replacement if cache is full
    cache[fifo_index].id = msg->id;
    cache[fifo_index].msg = *msg;
    cache[fifo_index].valid = 1;

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
    cache[victim].last_access = time(NULL); // Track the time
    printf("Message %d added to cache using Random Replacement at index %d.\n", msg->id, victim);


}

void add_to_cache_lru(Message* msg) {
    int index = -1;
    time_t oldest_time = time(NULL);

    // Find an empty slot or the least recently used one
    for (int i = 0; i < CACHE_SIZE; i++) {
        if (!cache[i].valid) {
            index = i;
            break;
        }
        if (cache[i].last_access < oldest_time) {
            oldest_time = cache[i].last_access;
            index = i;
        }
    }

    // Replace and update the cache
    cache[index].valid = 1;
    cache[index].id = msg->id;
    cache[index].msg = *msg;
    cache[index].last_access = time(NULL);
    printf("Message %d added to cache using LRU Replacement at index %d.\n", msg->id, index);
}


int store_msg(Message* msg) {
    int result = store_msg_to_disk(msg);
    if (result == 0) {
        add_to_cache(msg);
    }
    return result;
}

Message* retrieve_msg(int id) {
    Message* cached = check_cache(id);
    if (cached != NULL) {
        printf("[Cache HIT]\n");
        return cached;
    }
    
    printf("[Cache MISS]\n");
    Message* loaded = retrieve_msg_from_disk(id);
    if (loaded != NULL) {
        add_to_cache(loaded);
    }

    return loaded;
}



int main() {
    
    Message* msg = create_msg(1, "Prabesh", "Vanessa", "This is a test message.");
    store_msg(msg);

    Message* retrieved_msg = retrieve_msg(1);

    if (retrieved_msg != NULL) {
        printf("ID: %d\n", retrieved_msg->id);
        printf("Timestamp: %ld\n", retrieved_msg->timestamp);
        printf("Sender: %s\n", retrieved_msg->sender);
        printf("Receiver: %s\n", retrieved_msg->receiver);
        printf("Content: %s\n", retrieved_msg->content);
        printf("Delivered: %d\n", retrieved_msg->delivered);
    } else {
        printf("Failed to retrieve message.\n");
    }


    return 0;
}