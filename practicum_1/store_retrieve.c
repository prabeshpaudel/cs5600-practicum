#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct {
    int id;
    time_t timestamp;
    char sender[50];
    char receiver[50];
    char content[256];
    int delivered;
} Message;

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

int store_msg(Message* msg) {

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

Message* retrieve_msg(int id) {
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

int main() {
    Message* msg = create_msg(1, "Prabesh", "Vanessa", "This is a test message.");
    store_msg(msg);

    free(msg);

    Message* retrieved_msg = retrieve_msg(1);

    if (retrieved_msg != NULL) {
        printf("ID: %d\n", retrieved_msg->id);
        printf("Timestamp: %ld\n", retrieved_msg->timestamp);
        printf("Sender: %s\n", retrieved_msg->sender);
        printf("Receiver: %s\n", retrieved_msg->receiver);
        printf("Content: %s\n", retrieved_msg->content);
        printf("Delivered: %d\n", retrieved_msg->delivered);
        free(retrieved_msg);
    } else {
        printf("Failed to retrieve message.\n");
    }

    return 0;
}