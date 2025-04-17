#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUFFER_SIZE 8196
#define MAX_FILES 100


typedef struct {
    char path[1024];
    int is_readonly;  // 1 for read-only, 0 for read-write
} FilePermission;

FilePermission permissions[MAX_FILES];
int permission_count = 0;

// Check if a file is read-only
int is_file_readonly(const char *path) {
    for (int i = 0; i < permission_count; i++) {
        if (strcmp(permissions[i].path, path) == 0) {
            return permissions[i].is_readonly;
        }
    }
    return 0;  // Default to read-write if not found
}


void handle_writes(const char* remote_path, const char* file_data, const char* access) {
    char full_path[1024];
    snprintf(full_path, sizeof(full_path), "%s", remote_path);

    
    char folder[1024];
    strncpy(folder, full_path, sizeof(folder));
    char *last_slash = strrchr(folder, '/');
    if (last_slash) {
        *last_slash = '\0';
        char mkdir_cmd[1024];
        snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p \"%s\"", folder);
        system(mkdir_cmd);
    }
    // Check if file exists and is read-only
    if (is_file_readonly(full_path)) {
        perror("Cannot write to read-only file\n");
        return;
    }

    int found = 0;
    for (int i = 0; i < permission_count; i++) {
        if (strcmp(permissions[i].path, full_path) == 0) {
            found = 1;
            break;
        }
    }


        FILE *f = fopen(full_path, "w");
    if (f) {
        fwrite(file_data, 1, strlen(file_data), f);
        fclose(f);

        // Use default if access is NULL or empty
        const char *effective_access = (access && strlen(access) > 0) ? access : "RW";

        if (!found && permission_count < MAX_FILES) {
            strncpy(permissions[permission_count].path, full_path, sizeof(permissions[0].path));
            permissions[permission_count].is_readonly = (strcmp(effective_access, "RO") == 0);
            permission_count++;
        }

        printf("File written to %s as %s\n", full_path, 
               (strcmp(effective_access, "RO") == 0) ? "read-only" : "read-write");
    } else {
        printf("Failed to write to %s\n", full_path);
    }


}

void handle_gets(char *remote_path, int client_socket) {
    if (!remote_path) {
        send(client_socket, "ERROR: Missing remote path", 26, 0);
        return;
    }
    
    char full_path[BUFFER_SIZE];
    snprintf(full_path, BUFFER_SIZE, "%s", remote_path);
    printf("Full path: %s\n", full_path);
    FILE *file = fopen(full_path, "rb");
    if (!file) {
        send(client_socket, "ERROR: File not found", 21, 0);
        return;
    }
    
    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        send(client_socket, buffer, bytes_read, 0);
    }
    
    fclose(file);
}

void handle_rm(const char *path, int client_sock) {
    if (is_file_readonly(path)) {
            send(client_sock, "ERROR: File is read-only", 24, 0);
            return;
        }

    if (!path) {
        send(client_sock, "ERROR: No path specified", 24, 0);
        return;
    }

    if (remove(path) == 0) {
        send(client_sock, "SUCCESS: File removed", 22, 0);
        return;
    }

    if (rmdir(path) == 0) {
        send(client_sock, "SUCCESS: Folder removed", 24, 0);
        return;
    }

    perror("rm error");
    send(client_sock, "ERROR: Could not remove path", 28, 0);
}

void handle_client(int client_sock) {
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));


    int bytes_received = recv(client_sock, buffer, sizeof(buffer), 0);

    if (bytes_received <= 0) {
        perror("Receive failed");
        close(client_sock);
        return;
    }

    char *newline_pos = strchr(buffer, '\n');
    if (!newline_pos) {
        printf("Invalid message format (no newline)\n");
        close(client_sock);
        return;
    }

    *newline_pos = '\0';
    char *file_data = newline_pos + 1;

    char *command = strtok(buffer, " ");
    printf("command: %s\n", command);

 
    if (strcmp(command, "GET") == 0) {
        char *remote_path = strtok(NULL, "\n");
        printf("remote_path: %s\n", remote_path);
        handle_gets(remote_path, client_sock);
    } else if (strcmp(command, "WRITE") == 0) {
        if (strlen(file_data) == 0) {
            printf("Invalid WRITE format: missing file data\n");
            close(client_sock);
            return;
        }
        char *remote_path = strtok(NULL, " ");
        char *access = strtok(NULL, "\n"); // This gets the rest of the line before \n

        printf("remote_path: %s\n", remote_path);
        printf("access: %s\n", access);

        handle_writes(remote_path, file_data, access);
    } else if (strcmp(command, "RM") == 0) {
        char *remote_path = strtok(NULL, "\n");
        printf("remote_path: %s\n", remote_path);
        handle_rm(remote_path, client_sock);
    } else {
        printf("Unsupported command\n");
        send(client_sock, "ERROR: Unsupported command", 26, 0);
    }    

    close(client_sock);
}

int main(void) {
    int socket_desc, client_sock;
    socklen_t client_size;
    struct sockaddr_in server_addr, client_addr;

    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc < 0) {
        perror("Socket error");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(2000);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        return 1;
    }

    if (listen(socket_desc, 5) < 0) {
        perror("Listen failed");
        return 1;
    }

    printf("Server listening on port 2000...\n");

    while (1) {
        client_size = sizeof(client_addr);
        client_sock = accept(socket_desc, (struct sockaddr*)&client_addr, &client_size);
        if (client_sock < 0) {
            perror("Accept failed");
            continue;
        }

        printf("Client connected from %s:%d\n",
               inet_ntoa(client_addr.sin_addr),
               ntohs(client_addr.sin_port));

        handle_client(client_sock);
    }

    close(socket_desc);
    return 0;
}