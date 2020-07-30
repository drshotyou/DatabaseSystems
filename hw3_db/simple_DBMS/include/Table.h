#ifndef TABLE_H
#define TABLE_H
#include <stdlib.h>
#include <stdio.h>
#include "User.h"
#include "Command.h"


#define INIT_TABLE_SIZE 10000
#define EXT_LEN 500

typedef struct Table {
    size_t capacity;
    size_t len;
    User_t *users;
    unsigned char *cache_map;
    FILE *fp;
    char *file_name;
} Table_t;

Table_t *new_Table(char *file_name);
int add_User(Table_t *table, User_t *user);
int archive_table(Table_t *table);
void delete_users(Table_t *table, int *idxList, size_t idxListLen, Command_t *cmd);
int load_table(Table_t *table, char *file_name);
User_t* get_User(Table_t *table, size_t idx);
void update_user(User_t *user, updates_t *updates);
int update_users(Table_t *table, int *idxList, size_t idxListLen, Command_t *cmd);
#endif
