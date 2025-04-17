#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "polybius.h"

#define BUFFER_SIZE 8196

int handle_write(const char* local_path, const char* remote_path, const char* access, int sock, int encryption_enabled) {
    FILE *f = fopen(local_path, "r");
    if (!f) {
        perror("Failed to open local file");
        return 1;
    }

    char file_data[BUFFER_SIZE];
    size_t bytes_read = fread(file_data, 1, sizeof(file_data) - 1, f);
    file_data[bytes_read] = '\0';
    fclose(f);

    char message[BUFFER_SIZE];

    if (encryption_enabled) {
        Table table = initializeMatrix();
        char encrypted_data[BUFFER_SIZE];
        pbEncode(&table, file_data, encrypted_data);
        snprintf(message, sizeof(message), "WRITE %s %s\n%s", remote_path, access, encrypted_data);
    } else {
        snprintf(message, sizeof(message), "WRITE %s %s\n%s", remote_path, access, file_data);
    }

    if (send(sock, message, strlen(message), 0) < 0) {
        perror("Send failed");
        close(sock);
        return 1;
    }

    printf("File sent successfully.\n");
    close(sock);
    return 0;
}

int handle_get(const char *local_path, const char *remote_path, int sock, int encryption_enabled) {
    char message[BUFFER_SIZE];
    snprintf(message, sizeof(message), "GET %s\n", remote_path);

    if (send(sock, message, strlen(message), 0) < 0) {
        perror("Send failed");
        close(sock);
        return 1;
    }

    char buffer[BUFFER_SIZE];
    int bytes_received;
    char file_data[BUFFER_SIZE] = {0};

    while ((bytes_received = recv(sock, buffer, BUFFER_SIZE, 0)) > 0) {
        if (strncmp(buffer, "ERROR:", 6) == 0) {
            printf("Server error: %s\n", buffer);
            return 1;
        }
        strncat(file_data, buffer, bytes_received);
    }

    FILE *out = fopen(local_path, "wb");
    if (!out) {
        printf("Error: Could not write to file\n");
        return 1;
    }

    if (encryption_enabled) {
        Table table = initializeMatrix();
        char decrypted[BUFFER_SIZE];
        pbDecode(file_data, &table, decrypted);
        fwrite(decrypted, 1, strlen(decrypted), out);
    } else {
        fwrite(file_data, 1, strlen(file_data), out);
    }

    fclose(out);
    printf("File downloaded successfully\n");
    return 0;
}

int handle_rm(const char *remote_path, int sock) {
    char message[BUFFER_SIZE];
    snprintf(message, sizeof(message), "RM %s\n", remote_path);

    if (send(sock, message, strlen(message), 0) < 0) {
        perror("Send failed");
        return 1;
    }

    char response[BUFFER_SIZE];
    int bytes_received = recv(sock, response, sizeof(response) - 1, 0);
    if (bytes_received <= 0) {
        printf("Error receiving server response\n");
        return 1;
    }

    response[bytes_received] = '\0';
    printf("Server: %s\n", response);

    return (strncmp(response, "SUCCESS", 7) == 0) ? 0 : 1;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage:\n");
        printf("  %s WRITE <local_file> <remote_path> <access> [-enc]\n", argv[0]);
        printf("  %s GET <remote_path> <local_file> [-enc]\n", argv[0]);
        printf("  %s RM <remote_path>\n", argv[0]);
        return 1;
    }

    int encryption_enabled = 0;
    char *command = NULL;
    char *arg1 = NULL;
    char *arg2 = NULL;
    char *access = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "WRITE") == 0) {
            if (i + 3 >= argc) {
                printf("Usage: %s WRITE <local_file> <remote_path> <access> [-enc]\n", argv[0]);
                return 1;
            }
            command = "WRITE";
            arg1 = argv[i + 1];
            arg2 = argv[i + 2];
            access = argv[i + 3];
        } else if (strcmp(argv[i], "GET") == 0) {
            if (i + 2 >= argc) {
                printf("Usage: %s GET <remote_path> <local_file> [-enc]\n", argv[0]);
                return 1;
            }
            command = "GET";
            arg1 = argv[i + 1];
            arg2 = argv[i + 2];
        } else if (strcmp(argv[i], "RM") == 0) {
            if (i + 1 >= argc) {
                printf("Usage: %s RM <remote_path>\n", argv[0]);
                return 1;
            }
            command = "RM";
            arg1 = argv[i + 1];
        } else if (strcmp(argv[i], "-enc") == 0) {
            encryption_enabled = 1;
        }
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(2000);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return 1;
    }

    if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
        perror("Connect failed");
        close(sock);
        return 1;
    }

    if (strcmp(command, "WRITE") == 0) {
        return handle_write(arg1, arg2, access, sock, encryption_enabled);
    } else if (strcmp(command, "GET") == 0) {
        return handle_get(arg2, arg1, sock, encryption_enabled);
    } else if (strcmp(command, "RM") == 0) {
        return handle_rm(arg1, sock);
    }

    return 0;
}