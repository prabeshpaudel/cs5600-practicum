#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFER_SIZE 8196

int main(int argc, char *argv[]) {
    if (argc != 4 || strcmp(argv[1], "WRITE") != 0) {
        printf("Usage: %s WRITE <local_file> <remote_path>\n", argv[0]);
        return 1;
    }

    char *local_path = argv[2];
    char *remote_path = argv[3];

    FILE *f = fopen(local_path, "r");
    if (!f) {
        perror("Failed to open local file");
        return 1;
    }

    char file_data[BUFFER_SIZE];
    size_t bytes_read = fread(file_data, 1, sizeof(file_data) - 1, f);
    file_data[bytes_read] = '\0';
    fclose(f);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return 1;
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(2000);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
        perror("Connect failed");
        close(sock);
        return 1;
    }

    char message[BUFFER_SIZE];
    snprintf(message, sizeof(message), "WRITE %s\n%s", remote_path, file_data);

    if (send(sock, message, strlen(message), 0) < 0) {
        perror("Send failed");
        close(sock);
        return 1;
    }

    printf("File sent successfully.\n");

    close(sock);
    return 0;
}