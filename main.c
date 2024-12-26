#include <microhttpd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "database.h"

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

static enum MHD_Result request_handler(void *cls, struct MHD_Connection *connection, const char *url, const char *method, const char *version, const char *upload_data, size_t *upload_data_size, void **con_cls) {
    if (strcmp(method, "GET") == 0 && strcmp(url, "/users") == 0) {
        char *users = get_all_users();
        int ret = send_response(connection, users);
        free(users);
        return ret;
    } else if (strcmp(method, "POST") == 0 && strcmp(url, "/users") == 0) {
        if (*upload_data_size != 0) {
            *upload_data_size = 0; // marca os dados como processados
            return MHD_YES;
        }
// buff para armazenar nome e email
        char user_data[100] = {0}; 
        MHD_get_connection_values(connection, MHD_POSTDATA_KIND, post_iterator, user_data);

        char *name = user_data;
        char *email = user_data + 50;

        printf("Name: %s, Email: %s\n", name, email);
        return send_response(connection, "User created");
    }

    return send_response(connection, "Not found");
}

int main() {
    struct MHD_Daemon *daemon;

    daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, PORT, NULL, NULL, &request_handler, NULL, MHD_OPTION_END);
    if (!daemon) {
        fprintf(stderr, "Failed to start server\n");
        return 1;
    }

    printf("Server running on port %d\n", PORT);
    getchar();

    MHD_stop_daemon(daemon);
    close_db();

    return 0;
}