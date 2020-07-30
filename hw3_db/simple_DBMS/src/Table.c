#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "Table.h"
#include "Util.h"
///
/// Allocate a Table_t struct, then initialize some attributes, and
/// load table if the `file_name` is given
///
Table_t *new_Table(char *file_name) {
    Table_t *table = (Table_t*)malloc(sizeof(Table_t));
    memset((void*)table, 0, sizeof(Table_t));
    table->capacity = INIT_TABLE_SIZE;
    table->len = 0;
    table->users = (User_t*)malloc(
                            sizeof(User_t) * INIT_TABLE_SIZE);
    table->cache_map = (unsigned char*)malloc(sizeof(char)*INIT_TABLE_SIZE);
    memset(table->cache_map, 0, sizeof(char)*INIT_TABLE_SIZE);
    table->fp = NULL;
    table->file_name = NULL;
    load_table(table, file_name);
    return table;
}

///
/// Add the `User_t` data to the given table
/// If the table is full, it will allocate new space to store more
/// user data
/// return 1 when the data successfully add to table
///
int add_User(Table_t *table, User_t *user) {
    size_t idx;
    User_t *usr_ptr;
    if (!table || !user) {
        return 0;
    }
    // Check id doesn't exist in the table
    for (idx = 0; idx < table->len; idx++) {
        usr_ptr = get_User(table, idx);
        if (usr_ptr->id == user->id) {
            return 0;
        }
    }
    if (table->len == table->capacity) {
        User_t *new_user_buf = (User_t*)malloc(sizeof(User_t)*(table->len+EXT_LEN));
        unsigned char *new_cache_buf = (unsigned char *)malloc(sizeof(unsigned char)*(table->len+EXT_LEN));

        memcpy(new_user_buf, table->users, sizeof(User_t)*table->len);

        memset(new_cache_buf, 0, sizeof(unsigned char)*(table->len+EXT_LEN));
        memcpy(new_cache_buf, table->cache_map, sizeof(unsigned char)*table->len);


        free(table->users);
        free(table->cache_map);
        table->users = new_user_buf;
        table->cache_map = new_cache_buf;
        table->capacity += EXT_LEN;
    }
    idx = table->len;
    memcpy((table->users)+idx, user, sizeof(User_t));
    table->cache_map[idx] = 1;
    table->len++;
    //printf("%d %s %s %d",table->users[idx].id,table->users[idx].name,table->users[idx].email,table->users[idx].age);
    return 1;
}

///
/// Return value is the archived table len
///
int archive_table(Table_t *table) {
    size_t archived_len;
    struct stat st;

    if (table->fp == NULL) {
        return 0;
    }
    if (stat(table->file_name, &st) == 0) {
        archived_len = st.st_size / sizeof(User_t);
    } else {
        archived_len = 0;
    }
    fwrite((void*)(table->users+archived_len), \
            sizeof(User_t), table->len-archived_len, \
            table->fp);

    fclose(table->fp);
    free(table->file_name);
    table->fp = NULL;
    table->file_name = NULL;
    return table->len;
}

///
/// Loading the db file will overwrite the existed records in table,
/// only if the ``file_name`` is NULL
/// Return: the number of records in the db file
///
int load_table(Table_t *table, char *file_name) {
    size_t archived_len;
    struct stat st;
    if (table->fp != NULL) {
        fclose(table->fp);
        free(table->file_name);
        table->fp = NULL;
        table->file_name = NULL;
    }
    if (file_name != NULL) {
        table->len = 0;
        memset(table->cache_map, 0, sizeof(char)*INIT_TABLE_SIZE);
        if (stat(file_name, &st) != 0) {
            //Create new file
            table->fp = fopen(file_name, "wb");
        } else {
            archived_len = st.st_size / sizeof(User_t);
            if (archived_len > table->capacity) {
                User_t *new_user_buf = (User_t*)malloc(sizeof(User_t)*(archived_len+EXT_LEN));
                unsigned char *new_cache_buf = (unsigned char *)malloc(sizeof(unsigned char)*(archived_len+EXT_LEN));

                memset(new_cache_buf, 0, sizeof(unsigned char)*(archived_len+EXT_LEN));

                free(table->users);
                free(table->cache_map);
                table->users = new_user_buf;
                table->cache_map = new_cache_buf;
                table->capacity = archived_len+EXT_LEN;
            }
            table->fp = fopen(file_name, "a+b");
            table->len = archived_len;
        }
        table->file_name = strdup(file_name);
    }
    return table->len;
}

///
/// Return the user in table by the given index
///
User_t* get_User(Table_t *table, size_t idx) {
    size_t archived_len;
    struct stat st;
    if (!table->cache_map[idx]) {
        if (idx > INIT_TABLE_SIZE) {
            goto error;
        }
        if (stat(table->file_name, &st) != 0) {
            goto error;
        }
        archived_len = st.st_size / sizeof(User_t);
        if (idx >= archived_len) {
            //neither in file, nor in memory
            goto error;
        }

        fseek(table->fp, idx*sizeof(User_t), SEEK_SET);
        fread(table->users+idx, sizeof(User_t), 1, table->fp);
        table->cache_map[idx] = 1;
    }
    return table->users+idx;

error:
    return NULL;
}
//Here or on User.c?
void delete_users(Table_t *table, int *idxList, size_t idxListLen, Command_t *cmd) {
    size_t idx;
    idxListLen = set_idxlist(table, &idxList, idxListLen, cmd, 1);
    int len = 0;
    


    if (cmd->conds.where) {
        for (idx = 0; idx < idxListLen; idx++) 
        {
            table->users[len] = table->users[idxList[idx]];
            len ++;
        }
    } else {
        len = 0;
    }
    table->len = len;
}

void update_user(User_t *user, updates_t *updates) {
    if (!strncmp(updates->field, "id", 2)) {
        user->id = updates->ints;
    } else if (!strncmp(updates->field, "age", 3)) {
        user->age = updates->ints;
    } else if (!strncmp(updates->field, "name", 4)) {
        strcpy(user->name,updates->strs);
    } else if (!strncmp(updates->field, "email", 5)) {
        strcpy(user->email,updates->strs);
    }
}

int update_users(Table_t *table, int *idxList, size_t idxListLen, Command_t *cmd) {
    int ret = 1;
    size_t idx;
    int p = 1;

    idxListLen = set_idxlist(table, &idxList, idxListLen, cmd, 0);
    if (!strncmp(cmd->updates.field, "id", 2)) {
        if (idxListLen > 1) p = 0;
        for (idx = 0; idx < table->len; idx ++) {
            if (get_User(table, idx)->id == cmd->updates.ints)
                p = 0;
        }
    }

    if (cmd->conds.where && p==1) {
        for (idx = 0; idx < idxListLen; idx++)
        {
            update_user(get_User(table, idxList[idx]), &(cmd->updates));
        }
            
    } else if ((!cmd->conds.where && (strncmp(cmd->updates.field, "id", 2)) || table->len <= 1)) {
        for (idx = 0; idx < table->len; idx++) 
        {
            update_user(get_User(table, idx), &(cmd->updates));
        }
    } else {
        ret = 0;
    }
    return ret;
}
