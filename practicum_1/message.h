#ifndef MESSAGE_H
#define MESSAGE_H

#include <time.h>

int access_counter;

typedef struct {
    int id;
    time_t timestamp;
    char sender[50];
    char receiver[50];
    char content[256];
    int delivered;
} Message;
int cache_hits;
int cache_misses;

Message* create_msg(int id, const char* sender, const char* receiver, const char* content);
int store_msg(Message* msg, const char* method);
Message* retrieve_msg(int id,  const char* method);
void reset_hm ();



#endif // MESSAGE_H
