#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFER_SIZE 8196

int handle_write(const char* local_path, const char* remote_path, const char* access, int sock) {
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
    snprintf(message, sizeof(message), "WRITE %s %s\n%s", remote_path, access, file_data);

    if (send(sock, message, strlen(message), 0) < 0) {
        perror("Send failed");
        close(sock);
        return 1;
    }

     printf("File sent successfully.\n");
     close(sock);
     return 0;
}

int handle_get(const char *local_path, const char *remote_path, int sock) {
    
    char message[BUFFER_SIZE];
    snprintf(message, sizeof(message), "GET %s\n", remote_path);
    
    if (send(sock, message, strlen(message), 0) < 0) {
        perror("Send failed");
        close(sock);
        return 1;
    }

    FILE *file = fopen(local_path, "wb");
    if (!file) {
        printf("Error: Could not create local file\n");
        return 1;
    }
    
    char buffer[BUFFER_SIZE];
    int bytes_received;
    while ((bytes_received = recv(sock, buffer, BUFFER_SIZE, 0)) > 0) {
        if (strncmp(buffer, "ERROR:", 6) == 0) {
            printf("Server error: %s\n", buffer);
            fclose(file);
            remove(local_path);
            return 1;
        }
        fwrite(buffer, 1, bytes_received, file);
    }
    
    fclose(file);
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
    if ((strcmp(argv[1], "WRITE")) == 0 && (!(argc ==4||argc ==5)) ) {
        printf("  %s WRITE <local_file> <remote_path> <access>\n", argv[0]);
        return 1;
        
    } else if (strcmp(argv[1], "GET") == 0 && argc != 4) {
        printf("Usage:\n");
        printf("  %s GET <remote_path> <local_file>\n", argv[0]);


    }
    else if (strcmp(argv[1], "RM") == 0 && argc != 3) {
        printf("Usage:\n");
        printf("  %s RM <remote_path>\n", argv[0]);
        return 1;
    } else if (strcmp(argv[1], "WRITE") != 0 && strcmp(argv[1], "GET") != 0 && strcmp(argv[1], "RM") != 0) {
        printf("Unsupported command: %s\n", argv[1]);
        return 1;
    }    


    char *command = argv[1];
    char *arg1 = argv[2];
    char *arg2 = argv[3];
    char *access = argv[4];

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
        return handle_write(arg1, arg2, access, sock);
    } else if(strcmp(command, "GET") == 0) {
        return handle_get(arg2, arg1, sock);
    } else if (strcmp(command, "RM") == 0) {
        return handle_rm(arg1, sock);
    }
}