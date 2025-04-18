#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/file.h>  // For file locking

#define BUFFER_SIZE 8196
#define MAX_FILES 100

typedef struct {
    char path[1024];
    char is_readonly[3];  
} FilePermission;

// Shared data structures need protection
FilePermission permissions[MAX_FILES];
int permission_count = 0;
pthread_mutex_t permissions_mutex = PTHREAD_MUTEX_INITIALIZER;

// Structure to pass client data to threads
typedef struct {
    int sock;
    struct sockaddr_in address;
} client_data_t;

// Thread-safe file existence check
int file_exists(const char *path) {
    return access(path, F_OK) == 0;
}

// Thread-safe permission check with mutex protection
int is_file_readonly(const char *path) {
    pthread_mutex_lock(&permissions_mutex);
    for (int i = 0; i < permission_count; i++) {
        if (strcmp(permissions[i].path, path) == 0) {
            int result = (strcmp(permissions[i].is_readonly, "RO") == 0);
            pthread_mutex_unlock(&permissions_mutex);
            return result;
        }
    }
    pthread_mutex_unlock(&permissions_mutex);
    return 0;
}

// Thread-safe permission update
void update_permissions(const char *path, const char *access) {
    pthread_mutex_lock(&permissions_mutex);
    
    // Check if path already exists in permissions
    for (int i = 0; i < permission_count; i++) {
        if (strcmp(permissions[i].path, path) == 0) {
            strncpy(permissions[i].is_readonly, access, 2);
            permissions[i].is_readonly[2] = '\0';
            pthread_mutex_unlock(&permissions_mutex);
            return;
        }
    }
    
    // Add new permission if space available
    if (permission_count < MAX_FILES) {
        strncpy(permissions[permission_count].path, path, sizeof(permissions[0].path));
        strncpy(permissions[permission_count].is_readonly, access, 2);
        permissions[permission_count].is_readonly[2] = '\0';
        permission_count++;
    }
    
    pthread_mutex_unlock(&permissions_mutex);
}

// Lock a file for exclusive access
int lock_file(const char *path) {
    int fd = open(path, O_RDWR | O_CREAT, 0644);
    if (fd == -1) return -1;
    
    if (flock(fd, LOCK_EX) == -1) {
        close(fd);
        return -1;
    }
    
    return fd; // Caller must close this fd to release lock
}

// Release file lock
void unlock_file(int fd) {
    flock(fd, LOCK_UN);
    close(fd);
}

void handle_writes(const char* remote_path, const char* file_data, const char* access) {
    char full_path[1024];
    snprintf(full_path, sizeof(full_path), "%s", remote_path);
    
    // Create parent directories if needed
    char folder[1024];
    strncpy(folder, full_path, sizeof(folder));
    char *last_slash = strrchr(folder, '/');
    if (last_slash) {
        *last_slash = '\0';
        char mkdir_cmd[1024];
        snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p \"%s\"", folder);
        system(mkdir_cmd);
    }

    // Lock the file for exclusive access
    int fd = lock_file(full_path);
    if (fd == -1) {
        perror("Failed to lock file");
        return;
    }

    // Check if file exists and is read-only
    if (is_file_readonly(full_path)) {
        printf("Cannot write to read-only file: %s\n", full_path);
        unlock_file(fd);
        return;
    }

    // Write to file
    FILE *f = fopen(full_path, "w");
    if (f) {
        fwrite(file_data, 1, strlen(file_data), f);
        fclose(f);
        
        // Update permissions
        const char *effective_access = (access && strlen(access) > 0) ? access : "RW";
        update_permissions(full_path, effective_access);
        
        printf("File written to %s as %s\n", full_path,
               (strcmp(effective_access, "RO") == 0) ? "read-only" : "read-write");
    } else {
        printf("Failed to write to %s\n", full_path);
    }

    unlock_file(fd);
}

void handle_gets(char *remote_path, int client_socket) {
    if (!remote_path) {
        send(client_socket, "ERROR: Missing remote path", 26, 0);
        return;
    }
    
    char full_path[BUFFER_SIZE];
    snprintf(full_path, BUFFER_SIZE, "%s", remote_path);
    
    // Lock file for reading (shared lock)
    int fd = open(full_path, O_RDONLY);
    if (fd == -1) {
        send(client_socket, "ERROR: File not found", 21, 0);
        return;
    }
    
    if (flock(fd, LOCK_SH) == -1) {
        close(fd);
        send(client_socket, "ERROR: Could not lock file", 26, 0);
        return;
    }
    
    FILE *file = fdopen(fd, "rb");
    if (!file) {
        close(fd);
        send(client_socket, "ERROR: Could not open file", 26, 0);
        return;
    }
    
    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        send(client_socket, buffer, bytes_read, 0);
    }
    
    fclose(file); // This also closes fd and releases lock
}

void handle_rm(const char *path, int client_sock) {
    if (is_file_readonly(path)) {
        send(client_sock, "ERROR: File is read-only", 24, 0);
        return;
    }

    // Lock the file/directory before removal
    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        send(client_sock, "ERROR: Path not found", 21, 0);
        return;
    }
    
    if (flock(fd, LOCK_EX) == -1) {
        close(fd);
        send(client_sock, "ERROR: Could not lock path", 26, 0);
        return;
    }

    int result;
    if (remove(path) == 0) {
        result = 1; // File removed
    } else if (rmdir(path) == 0) {
        result = 2; // Directory removed
    } else {
        result = 0; // Failed
    }

    // Release lock before checking result
    flock(fd, LOCK_UN);
    close(fd);

    if (result == 1) {
        send(client_sock, "SUCCESS: File removed", 22, 0);
    } else if (result == 2) {
        send(client_sock, "SUCCESS: Folder removed", 24, 0);
    } else {
        send(client_sock, "ERROR: Could not remove path", 28, 0);
    }
}

void* handle_client_thread(void *arg) {
    client_data_t *client_data = (client_data_t*)arg;
    int client_sock = client_data->sock;
    
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));

    int bytes_received = recv(client_sock, buffer, sizeof(buffer), 0);

    if (bytes_received <= 0) {
        perror("Receive failed");
        close(client_sock);
        free(client_data);
        return NULL;
    }

    char *newline_pos = strchr(buffer, '\n');
    if (!newline_pos) {
        printf("Invalid message format (no newline)\n");
        close(client_sock);
        free(client_data);
        return NULL;
    }

    *newline_pos = '\0';
    char *file_data = newline_pos + 1;

    char *command = strtok(buffer, " ");
    printf("[Thread %lu] Command: %s\n", pthread_self(), command);

    if (strcmp(command, "GET") == 0) {
        char *remote_path = strtok(NULL, "\n");
        printf("[Thread %lu] GET: %s\n", pthread_self(), remote_path);
        handle_gets(remote_path, client_sock);
    } else if (strcmp(command, "WRITE") == 0) {
        if (strlen(file_data) == 0) {
            printf("Invalid WRITE format: missing file data\n");
            close(client_sock);
            free(client_data);
            return NULL;
        }
        char *remote_path = strtok(NULL, " ");
        char *access = strtok(NULL, "\n");

        printf("[Thread %lu] WRITE: %s with access %s\n", 
               pthread_self(), remote_path, access);
        handle_writes(remote_path, file_data, access);
    } else if (strcmp(command, "RM") == 0) {
        char *remote_path = strtok(NULL, "\n");
        printf("[Thread %lu] RM: %s\n", pthread_self(), remote_path);
        handle_rm(remote_path, client_sock);
    } else {
        printf("Unsupported command\n");
        send(client_sock, "ERROR: Unsupported command", 26, 0);
    }    

    close(client_sock);
    free(client_data);
    return NULL;
}

int main(void) {
    int socket_desc;
    struct sockaddr_in server_addr;

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
        client_data_t *client_data = malloc(sizeof(client_data_t));
        if (!client_data) {
            perror("malloc failed");
            continue;
        }

        socklen_t client_size = sizeof(client_data->address);
        client_data->sock = accept(socket_desc, 
                                 (struct sockaddr*)&client_data->address,
                                 &client_size);
        if (client_data->sock < 0) {
            perror("Accept failed");
            free(client_data);
            continue;
        }

        printf("Client connected from %s:%d\n",
               inet_ntoa(client_data->address.sin_addr),
               ntohs(client_data->address.sin_port));

        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_client_thread, (void*)client_data) != 0) {
            perror("Thread creation failed");
            close(client_data->sock);
            free(client_data);
        }
        
        // Detach the thread so we don't need to join it
        pthread_detach(thread_id);
    }

    close(socket_desc);
    return 0;
}