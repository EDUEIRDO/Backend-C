#include <microhttpd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "database.h"
#include <stdarg.h>
#include <time.h>

#define PORT 8888

static int send_response(struct MHD_Connection *connection, const char *message) {
    struct MHD_Response *response;
    int ret;

    response = MHD_create_response_from_buffer(strlen(message), (void *)message, MHD_RESPMEM_PERSISTENT);
    if (!response) return MHD_NO;

    ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);
    return ret;
}

static enum MHD_Result post_iterator(void *cls, enum MHD_ValueKind kind, const char *key, const char *value) {
    char *data = (char *)cls;
    if (strcmp(key, "name") == 0) {
        strncpy(data, value, 49);
    } else if (strcmp(key, "email") == 0) {
        strncpy(data + 50, value, 49);
    }
    return MHD_YES;
}

// Função de logs do servidor
void log_message(const char *format, ...) {
    FILE *log_file = fopen("server.log", "a");
    if (!log_file) return;

    va_list args;
    va_start(args, format);

    // Adicionar timestamp no log
    time_t now = time(NULL);
    char *timestamp = ctime(&now);
    timestamp[strlen(timestamp) - 1] = '\0'; // Remover newline

    fprintf(log_file, "[%s] ", timestamp);
    vfprintf(log_file, format, args);
    fprintf(log_file, "\n");

    va_end(args);
    fclose(log_file);
}

static enum MHD_Result request_handler(void *cls, struct MHD_Connection *connection, const char *url, const char *method, const char *version, const char *upload_data, size_t *upload_data_size, void **con_cls) {
    static char user_data[100] = {0}; // Buffer para armazenar nome e email
    clock_t start = clock();

    if (strcmp(method, "GET") == 0 && strcmp(url, "/users") == 0) {
        log_message("Received GET request to /users");
        char *users = get_all_users();
        int ret = send_response(connection, users);
        free(users);

        clock_t end = clock();
        double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
        log_message("GET /users processed in %.6f seconds", elapsed);

        return ret;
    } else if (strcmp(method, "POST") == 0 && strcmp(url, "/users") == 0) {
        if (*upload_data_size != 0) {
            MHD_get_connection_values(connection, MHD_POSTDATA_KIND, post_iterator, user_data);
            *upload_data_size = 0;
            return MHD_YES;
        } else {
            char *name = user_data;
            char *email = user_data + 50;
            log_message("Received POST request to /users with Name=%s, Email=%s", name, email);

            create_user(name, email);

            clock_t end = clock();
            double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
            log_message("POST /users processed in %.6f seconds", elapsed);

            return send_response(connection, "User created");
        }
    }

    return send_response(connection, "Not found");
}

int main() {
    if (!init_db()) {
        fprintf(stderr, "Database initialization failed\n");
        return 1;
    }
    struct MHD_Daemon *daemon;

    daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, PORT, NULL, NULL, &request_handler, NULL, MHD_OPTION_END);
    if (!daemon) {
        fprintf(stderr, "Failed to start server\n");
        return 1;
    }

    printf("Server running on port %d\n", PORT);
    log_message("Server started on port %d", PORT);

    getchar(); // Press Enter to stop the server

    MHD_stop_daemon(daemon);
    close_db();

    return 0;
}