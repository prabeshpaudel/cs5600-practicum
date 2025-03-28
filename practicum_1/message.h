#ifndef MESSAGE_H
#define MESSAGE_H

#include <time.h>

int access_counter;  // tracks total message accesses (used for cache metrics)

typedef struct {
    int id;                  // unique message ID
    time_t timestamp;        // time the message was created
    char sender[50];         // sender identifier
    char receiver[50];       // receiver identifier
    char content[256];       // message content
    int delivered;           // delivery status flag
} Message;

int cache_hits;             // number of cache hits during retrieval
int cache_misses;           // number of cache misses during retrieval

// creates a new message with given details
Message* create_msg(int id, const char* sender, const char* receiver, const char* content);

// stores the message using specified method ("disk", "cache", etc.)
int store_msg(Message* msg, const char* method);

// retrieves message by ID using specified method
Message* retrieve_msg(int id, const char* method);

// resets internal hashmap or related data structures
void reset_hm();

#endif // MESSAGE_H