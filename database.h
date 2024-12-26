#ifndef DATABASE_H
#define DATABASE_H

int init_db();
void close_db();
void create_user(const char *name, const char *email);
char* get_all_users();

#endif