#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cache.h"
#include "message.h"

#define NUM_MESSAGES 1000
#define NUM_ACCESSES 1000



void generate_messages(const char* method) {
    reset_cache();
    for (int i = 0; i < NUM_MESSAGES; i++) {
        char sender[50], receiver[50], content[256];
        snprintf(sender, sizeof(sender), "Sender_%d", i);
        snprintf(receiver, sizeof(receiver), "Receiver_%d", i);
        snprintf(content, sizeof(content), "This is message %d", i);

        Message* msg = create_msg(i, sender, receiver, content);
        store_msg(msg, method);
        free(msg);
    }
    printf("Generated %d messages.\n", NUM_MESSAGES);


   
    //reset_hm();

}

void evaluate_cache(const char* method) {

    for (int i = 0; i < NUM_ACCESSES; i++) {
        int msg_id = rand() % NUM_MESSAGES;
        retrieve_msg(msg_id, method);
    }

    double hit_ratio = (double)cache_hits / NUM_ACCESSES;
    printf("Cache Hits: %d, Cache Misses: %d, Hit Ratio: %.2f\n", cache_hits, cache_misses, hit_ratio);
    reset_cache();
    reset_hm();
}


int main(int argc, char* argv[]) {
    srand(time(NULL));

    generate_messages(argv[1]);
    evaluate_cache(argv[1]);
    return 0;

}


