#include <stdio.h>
#include <stdio_ext.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include "Util.h"
#include "Command.h"
#include "Table.h"
#include "SelectState.h"

///
/// Allocate State_t and initialize some attributes
/// Return: ptr of new State_t
///
State_t* new_State() {
    State_t *state = (State_t*)malloc(sizeof(State_t));
    state->saved_stdout = -1;
    return state;
}

///
/// Print shell prompt
///
void print_prompt(State_t *state) {
    if (state->saved_stdout == -1) {
        printf("db > ");
    }
}

///
/// Print the user in the specific format
///select only
void print_user(User_t *user, SelectArgs_t *sel_args) {
    size_t idx;
    //size_t i=0,j=1;
    //int *checks=malloc((int)5);
    //printf("print user 1\n");
    printf("(");
    for (idx = 0; idx < sel_args->fields_len; idx++) {
        if (!strncmp(sel_args->fields[idx], "*", 1)) {
            printf("%d, %s, %s, %d", user->id, user->name, user->email, user->age);
        } else {
            if (idx > 0) printf(", ");

            if (!strncmp(sel_args->fields[idx], "id", 2)) {
                printf("%d", user->id);
            } else if (!strncmp(sel_args->fields[idx], "name", 4)) {
                printf("%s", user->name);
            } else if (!strncmp(sel_args->fields[idx], "email", 5)) {
                printf("%s", user->email);
            } else if (!strncmp(sel_args->fields[idx], "age", 3)) {
                printf("%d", user->age);
            }
        }
    }
    printf(")\n");
}
//Aggregates
void print_aggregateFunctions(SelectArgs_t *sel_args, aggregate_t *aggs)
{
    size_t idx;
    printf("(");
    for (idx = 0; idx < sel_args->fields_len; idx++) {
        if (idx > 0) printf(", ");

        if (!strncmp(sel_args->fields[idx], "sum(id)", 7)) {
            printf("%d", aggs->idTotal);
        } else if (!strncmp(sel_args->fields[idx], "sum(age)", 8)) {
            printf("%d", aggs->ageTotal);
        } else if (!strncmp(sel_args->fields[idx], "avg(id)", 7)) {
            printf("%.3lf", aggs->idCalc);
        } else if (!strncmp(sel_args->fields[idx], "avg(age)", 8)) {
            printf("%.3lf", aggs->ageCalc);
        } else if (!strncmp(sel_args->fields[idx], "count", 5)) {
            printf("%d", aggs->countTotal);
        }
    }
    printf(")\n");
}


///
/// Print the users for given offset and limit restriction
///
void print_users(Table_t *table, int *idxList, size_t idxListLen, Command_t *cmd) {
    size_t idx;
    int limit = cmd->cmd_args.sel_args.limit;
    int offset = cmd->cmd_args.sel_args.offset;
    //printf("Limit: %d offset: %d\n",limit ,offset);
    if (offset == -1) {
        offset = 0;
    }

    idxListLen = set_idxlist(table, &idxList, idxListLen, cmd, 0);
    //printf("len: %d\n",idxListLen);
    //printf("printUsers 1");
    if (cmd->aggs.aggre) 
    {
        //printf("COMES TO agre\n"); 
        cmd->aggs.idTotal = 0;
        cmd->aggs.ageTotal = 0;
        cmd->aggs.idCalc = 0;
        cmd->aggs.ageCalc = 0;
        cmd->aggs.countTotal = 0;
        if (cmd->conds.where) 
        {
            for (idx = 0; idx < idxListLen; idx++) {
                User_t *user = get_User(table, idxList[idx]);
                cmd->aggs.idTotal += user->id;
                cmd->aggs.ageTotal += user->age;
                cmd->aggs.idCalc += user->id;
                cmd->aggs.ageCalc += user->age;
            }
            if (idxListLen) cmd->aggs.idCalc /= idxListLen;
            if (idxListLen) cmd->aggs.ageCalc /= idxListLen;
            cmd->aggs.countTotal = idxListLen;
        } else {
            for (idx = 0; idx < table->len; idx++) {
                User_t *user = get_User(table, idx);
                cmd->aggs.idTotal += user->id;
                cmd->aggs.ageTotal += user->age;
                cmd->aggs.idCalc += user->id;
                cmd->aggs.ageCalc += user->age;
            }
            if (idxListLen) cmd->aggs.idCalc /= idxListLen;
            if (idxListLen) cmd->aggs.ageCalc /= idxListLen;
            cmd->aggs.countTotal = idxListLen;
        }
        if (offset == 0 && (limit > 0 || limit == -1))
            print_aggregateFunctions(&(cmd->cmd_args.sel_args), &(cmd->aggs));
    } else { //leave as is
        if (cmd->conds.where) {
            //printf("COMES TO WHERE\n");
            //printf("%d %d",idx, idxListLen);
            for (idx = offset; idx < idxListLen; idx++) {
                if (limit != -1 && (idx - offset) >= limit) {
                    //printf("BREAK\n");
                    break;
                }
                //printf("%d",idx);
                print_user(get_User(table, idxList[idx]), &(cmd->cmd_args.sel_args));
            }
        } else {
            //printf("COMES TO PRINTUSER\n");
            for (idx = offset; idx < table->len; idx++) {
                if (limit != -1 && (idx - offset) >= limit) {
                    break;
                }
                print_user(get_User(table, idx), &(cmd->cmd_args.sel_args));
            }
        }
    }
}

///
/// This function received an output argument
/// Return: category of the command
///
int parse_input(char *input, Command_t *cmd) {
    char *token;
    int idx;
    //printf("ParseInput begin %d\n",cmd->type);
    token = strtok(input, " ,\n");
    //printf("token: %s\n",token);
    for (idx = 0; strlen(cmd_list[idx].name) != 0; idx++) {
        if (!strncmp(token, cmd_list[idx].name, cmd_list[idx].len)) {
            cmd->type = cmd_list[idx].type;
            //printf("InFOr %d\n",cmd->type);
        }
    }
    while (token != NULL) {
        add_Arg(cmd, token);
        token = strtok(NULL, " ,\n");
        //printf("token: %s\n",token);
    }
    //printf("ParseInput end %d",cmd->type);
    return cmd->type;

}

///
/// Handle built-in commands
/// Return: command type
///
void handle_builtin_cmd(Table_t *table, Command_t *cmd, State_t *state) {
    if (!strncmp(cmd->args[0], ".exit", 5)) {
        archive_table(table);
        exit(0);
    } else if (!strncmp(cmd->args[0], ".output", 7)) {
        if (cmd->args_len == 2) {
            if (!strncmp(cmd->args[1], "stdout", 6)) {
                close(1);
                dup2(state->saved_stdout, 1);
                state->saved_stdout = -1;
            } else if (state->saved_stdout == -1) {
                int fd = creat(cmd->args[1], 0644);
                state->saved_stdout = dup(1);
                if (dup2(fd, 1) == -1) {
                    state->saved_stdout = -1;
                }
                __fpurge(stdout); //This is used to clear the stdout buffer
            }
        }
    } else if (!strncmp(cmd->args[0], ".load", 5)) {
        if (cmd->args_len == 2) {
            load_table(table, cmd->args[1]);
        }
    } else if (!strncmp(cmd->args[0], ".help", 5)) {
        print_help_msg();
    }
}

///
/// Handle query type commands
/// Return: command type
///
int handle_query_cmd(Table_t *table, Command_t *cmd) {
    if (!strncmp(cmd->args[0], "insert", 6)) {
        handle_insert_cmd(table, cmd);
        return INSERT_CMD;
    } else if (!strncmp(cmd->args[0], "select", 6)) {
        handle_select_cmd(table, cmd);
        return SELECT_CMD;
    } else if (!strncmp(cmd->args[0], "update", 6)) {
        handle_update_cmd(table, cmd);
        return SELECT_CMD;
    } else if (!strncmp(cmd->args[0], "delete", 6)) {
        handle_delete_cmd(table, cmd);
        return SELECT_CMD;
    } else {
        return UNRECOG_CMD;
    }
}

///
/// The return value is the number of rows insert into table
/// If the insert operation success, then change the input arg
/// `cmd->type` to INSERT_CMD
///
int handle_insert_cmd(Table_t *table, Command_t *cmd) {
    int ret = 0;
    User_t *user = command_to_User(cmd);
    if (user) {
        ret = add_User(table, user);
        if (ret > 0) {
            cmd->type = INSERT_CMD;
        }
    }
    return ret;
}

int handle_update_cmd(Table_t *table, Command_t *cmd) {
    int ret = 0;
    set_state_handler(cmd, 3);
    update_users(table, NULL, 0, cmd);
    return ret;
}

int handle_delete_cmd(Table_t *table, Command_t *cmd) {
    int ret = 0;
    if (cmd->args_len > 3 && !strncmp(cmd->args[3], "where", 5))
        where_state_handler(cmd, 4);

    delete_users(table, NULL, 0, cmd);
    return ret;
}
///
/// The return value is the number of rows select from table
/// If the select operation success, then change the input arg
/// `cmd->type` to SELECT_CMD
///
int handle_select_cmd(Table_t *table, Command_t *cmd) {
   // printf("HSC\n");
    cmd->type = SELECT_CMD;
   // printf("HSC cmd:%d\n",cmd->type);
    field_state_handler(cmd, 1);

    print_users(table, NULL, 0, cmd);
    return table->len;
}
size_t set_idxlist(Table_t *table, int **idxList, size_t idxListLen, Command_t *cmd, int state) {
    size_t idxListCap = idxListLen;
    for (int i = 0; i < table->len; i ++) {
        User_t* user = get_User(table, i);

        if((condsSat(cmd, user)) ^state) {        
            if (idxListCap==0 || idxListCap==idxListLen) {
                int *new_buf = (int*) malloc( sizeof(int) * (idxListCap + 5) );
                memset(new_buf, 0, sizeof(int) * (idxListCap + 5));
                memcpy(new_buf, *idxList, sizeof(int) * (idxListCap));
                free(*idxList);
                *idxList = new_buf;
                idxListCap += 5;
            }
            (*idxList)[idxListLen] = i;
            idxListLen ++;
        }
    }
    return idxListLen;
}

int condsSat(Command_t *cmd, User_t *user)
{
    int ans,and;
    if(cmd->conds.logOp == 1)
    {
        and=1;
        ans=1;
    } else
    {
        and=0;
        ans=0;
    }
    int tf;
    for(int i=0;i<cmd->conds.stringType_len;i++)
    {
        char *cond1;
        char *cond2 = cmd->conds.condsStr[2*i+1];
        if(!strncmp(cmd->conds.condsStr[2*i],"name",4))
        {
            cond1 = user->name;
        }
        else
        {
            cond1 = user->email;
        }
        tf=0;
        int op=cmd->conds.strCond[i];
        if(op==1 && (!strcmp(cond1,cond2))) tf=1;
        else if(op==2 && (strcmp(cond1,cond2))) tf=1;
        //printf("ans:%d and:%d tf:%d cond1:%s cond2:%s op:%d\n",ans, and, tf, cond1, cond2, op);
        if(and==1) ans=ans && tf;
        else ans = ans || tf;
    }
    for(int i=0;i<cmd->conds.numType_len;i++)
    {
        int cond1;
        int cond2 = cmd->conds.intOp[i];
        if(!strncmp(cmd->conds.condsNum[i],"id",2))
        {
            cond1 = user->id;
        } else {
            cond1=user->age;
        }
        int op = cmd->conds.numCond[i];
        tf=0;
        if(op==0 && cond1 == cond2) tf=1;
        else if(op==1 && cond1 != cond2) tf=1;
        else if(op==2 && cond1 > cond2) tf=1;
        else if(op==3 && cond1 < cond2) tf=1;
        else if(op==4 && cond1 >= cond2) tf=1;
        else if(op==5 && cond1 <= cond2) tf=1;
        //printf("ans:%d and:%d tf:%d cond1:%d cond2:%d op:%d",ans, and, tf, cond1, cond2, op);
        if(and==1) ans=ans && tf;
        else ans = ans || tf;
    }
    return ans;
}
///
/// Show the help messages
///
void print_help_msg() {
    const char msg[] = "# Supported Commands\n"
    "\n"
    "## Built-in Commands\n"
    "\n"
    "  * .exit\n"
    "\tThis cmd archives the table, if the db file is specified, then exit.\n"
    "\n"
    "  * .output\n"
    "\tThis cmd change the output strategy, default is stdout.\n"
    "\n"
    "\tUsage:\n"
    "\t    .output (<file>|stdout)\n\n"
    "\tThe results will be redirected to <file> if specified, otherwise they will display to stdout.\n"
    "\n"
    "  * .load\n"
    "\tThis command loads records stored in <DB file>.\n"
    "\n"
    "\t*** Warning: This command will overwrite the records already stored in current table. ***\n"
    "\n"
    "\tUsage:\n"
    "\t    .load <DB file>\n\n"
    "\n"
    "  * .help\n"
    "\tThis cmd displays the help messages.\n"
    "\n"
    "## Query Commands\n"
    "\n"
    "  * insert\n"
    "\tThis cmd inserts one user record into table.\n"
    "\n"
    "\tUsage:\n"
    "\t    insert <id> <name> <email> <age>\n"
    "\n"
    "\t** Notice: The <name> & <email> are string without any whitespace character, and maximum length of them is 255. **\n"
    "\n"
    "  * select\n"
    "\tThis cmd will display all user records in the table.\n"
    "\n";
    printf("%s", msg);
}

