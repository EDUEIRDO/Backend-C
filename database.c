#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "database.h"

static sqlite3 *db;

int init_db() {
    int rc = sqlite3_open("users.db", &db);
    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return 0;
    }

    const char *sql = "CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY, name TEXT, email TEXT);";
    char *err_msg = 0;
    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return 0;
    }
    return 1;
}

void close_db() {
    sqlite3_close(db);
}

void create_user(const char *name, const char *email) {
    const char *sql = "INSERT INTO users (name, email) VALUES (?, ?);";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, email, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Execution failed: %s\n", sqlite3_errmsg(db));
    }
    sqlite3_finalize(stmt);
}

char *get_all_users() {
    const char *sql = "SELECT * FROM users;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to fetch data: %s\n", sqlite3_errmsg(db));
        return NULL;
    }

    size_t buffer_size = 1024;
    char *result = malloc(buffer_size);
    if (!result) return NULL;
    strcpy(result, "[");

    int first = 1;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        char user[256];
        snprintf(user, sizeof(user), "{\"id\":%d,\"name\":\"%s\",\"email\":\"%s\"}",
                 sqlite3_column_int(stmt, 0),
                 sqlite3_column_text(stmt, 1),
                 sqlite3_column_text(stmt, 2));

        size_t new_length = strlen(result) + strlen(user) + 2;
        if (new_length > buffer_size) {
            buffer_size *= 2;
            result = realloc(result, buffer_size);
            if (!result) return NULL;
        }

        if (!first) strcat(result, ",");
        first = 0;
        strcat(result, user);
    }

    strcat(result, "]");
    sqlite3_finalize(stmt);
    return result;
}

