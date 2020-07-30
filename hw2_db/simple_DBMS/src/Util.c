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
///
void print_user(User_t *user) {
    printf("(%d, %s, %s, %d)\n", user->id, user->name, user->email, user->age);
}

///
/// This function received an output argument
/// Return: category of the command
///
int parse_input(char *input, Command_t *cmd) {
    char *token;
    int idx;
    token = strtok(input, " \n");
    for (idx = 0; strlen(cmd_list[idx].name) != 0; idx++) {
        if (!strncmp(token, cmd_list[idx].name, cmd_list[idx].len)) {
            cmd->type = cmd_list[idx].type;
        }
    }
    while (token != NULL) {
        add_Arg(cmd, token);
        token = strtok(NULL, " \n");
    }
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

///
/// The return value is the number of rows select from table
/// If the select operation success, then change the input arg
/// `cmd->type` to SELECT_CMD
///
int handle_select_cmd(Table_t *table, Command_t *cmd) {
    size_t idx;
    
            int val[4]={0,0,0,0};
            int off=0;
            int offPos=0;
            int lim=0;
            int limPos=0;
            int fields=0;
            int fieldSel=0;
            
            for(int i=1;i<cmd->args_len;i++)
            {
                 if(strncmp(cmd->args[i],"from",4) && (!strncmp(cmd->args[i],"id",2) || !strncmp(cmd->args[i],"name",4) || !strncmp(cmd->args[i],"email",5) ||  !strncmp(cmd->args[i],"*",1)))
                 {
                    
                     
                     if(!strncmp(cmd->args[i],"id",2))
                     {
                         fieldSel=1;
                         val[0]=1;
                         fields++;
                     }
                     else if(!strncmp(cmd->args[i],"name",4))
                     {
                         fieldSel=1;
                         val[1]=1;
                         fields++;
                     }
                     else if(!strncmp(cmd->args[i],"email",5))
                     {
                         fieldSel=1;
                         val[2]=1;
                         fields++;
                     }
                     else if(!strncmp(cmd->args[i],"age",3))
                     {
                         fieldSel=1;
                         val[3]=1;
                         fields++;
                     }
                     else if(!strncmp(cmd->args[i],"*",1))
                     {
                         fieldSel=1;
                         fields=4;
                         val[0]=1;
                         val[1]=1;
                         val[2]=1;
                         val[3]=1;
                     }
                 }
                 else if(strncmp(cmd->args[i],"id",2) || strncmp(cmd->args[i],"name",4) || strncmp(cmd->args[i],"email",5) || strncmp(cmd->args[i],"age",3))
                 {
                     if(!strncmp(cmd->args[i],"from",4))
                     {
                         continue;
                     }
                     else if(strncmp(cmd->args[i],"offset",6) && strncmp(cmd->args[i],"limit",5) && !strncmp(cmd->args[i-1],"from",4))
                     {
                         Table_t * newTable = new_Table(cmd->args[i]);
                     }
                     else if(!strncmp(cmd->args[i],"offset",6) || !strncmp(cmd->args[i],"limit",5))
                     {
                         if(!strncmp(cmd->args[i],"offset",6))
                         {
                            off=1;
                            offPos=i+1;
                         }
                         else if(!strncmp(cmd->args[i],"limit",5))
                         {
                             lim=1;
                             limPos=i+1;
                         }
                     }
                     
                     
                 }
            }//end processing

            if(fieldSel==1)
            {
                
                if(off==1 && lim==0)
                {
                    size_t temp=atoi(cmd->args[offPos]);
                    for (idx = temp; idx < table->len; idx++) {
                        printf("(");
                        User_t * tempU=get_User(table, idx);
                        int fieldt=0;
                        if(val[0]==1)
                        {
                            printf("%d",tempU->id);
                                  
                        }
                        if(fieldt<fields-1) printf(", ");
                        if(val[1]==1)
                        {
                            printf("%s",tempU->name);
                            fieldt++;          
                        }
                        if(fieldt<fields-1) printf(", ");
                        if(val[2]==1)
                        {
                            printf("%s",tempU->email);
                            fieldt++;          
                        }
                        if(fieldt<fields-1) printf(", ");
                        if(val[3]==1)
                        {
                            printf("%d",tempU->age);
                            fieldt++;          
                        }
                        printf(")\n");
                    }
                    
                }
                else if (lim==1 && off==0)
                {
                    size_t temp=atoi(cmd->args[limPos]);
                    for (idx = 0; idx < temp; idx++) {
                        printf("(");
                         User_t * tempU=get_User(table, idx);
                        int fieldt=0;
                        if(val[0]==1)
                        {
                            printf("%d",tempU->id);
                                    
                        }
                        if(fieldt<fields-1)
                        {
                            printf(", ");
                            fieldt++;  
                        } 
                        if(val[1]==1)
                        {
                            printf("%s",tempU->name);        
                        }
                        if(fieldt<fields-1){
                            printf(", ");
                            fieldt++;  
                        } 
                        if(val[2]==1)
                        {
                            printf("%s",tempU->email);        
                        }
                        if(fieldt<fields-1){
                            printf(", ");
                            fieldt++;  
                        } 
                        if(val[3]==1)
                        {
                            printf("%d",tempU->age);        
                        }
                        printf(")\n");
                    }

                    
                }
                else if(off==0 && lim==0)
                {
                    for (idx = 0; idx < table->len; idx++) {
                        printf("(");
                         User_t * tempU=get_User(table, idx);
                          int fieldt=0;
                        if(val[0]==1)
                        {
                            printf("%d",tempU->id);        
                        }
                        if(fieldt<fields-1){
                            printf(", ");
                            fieldt++;  
                        } 
                        if(val[1]==1)
                        {
                            printf("%s",tempU->name);        
                        }
                        if(fieldt<fields-1){
                            printf(", ");
                            fieldt++;  
                        } 
                        if(val[2]==1)
                        {
                            printf("%s",tempU->email);        
                        }
                        if(fieldt<fields-1) {
                            printf(", ");
                            fieldt++;  
                        } 
                        if(val[3]==1)
                        {
                            printf("%d",tempU->age);        
                        }
                        printf(")\n");
                    }
                    
                }
                else if(off==1 && lim==1)
                {
                    size_t temp=atoi(cmd->args[limPos]);
                    size_t temp1=atoi(cmd->args[offPos]);
                    for (idx = temp1; idx < temp; idx++) {
                        printf("(");
                         User_t * tempU=get_User(table, idx);
                          int fieldt=0;
                        if(val[0]==1)
                        {
                            printf("%d",tempU->id);        
                        }
                        if(fieldt<fields-1){
                            printf(", ");
                            fieldt++;  
                        } 
                        if(val[1]==1)
                        {
                            printf("%s",tempU->name);        
                        }
                        if(fieldt<fields-1){
                            printf(", ");
                            fieldt++;  
                        } 
                        if(val[2]==1)
                        {
                            printf("%s",tempU->email);        
                        }
                        if(fieldt<fields-1) {
                            printf(", ");
                            fieldt++;  
                        } 
                        if(val[3]==1)
                        {
                            printf("%d",tempU->age);        
                        }
                        printf(")\n");
                    }
                }
                
            }
            else if(fieldSel==0)
            {
                if(!strncmp(cmd->args[0],"select",6) && cmd->args[1]==NULL) 
                {   
                    for (idx = 0; idx < table->len; idx++) {
                        print_user(get_User(table, idx));
                    }
                }
                else if(!strncmp(cmd->args[1],"limit",5) && cmd->args[2])
                {
                    size_t temp=atoi(cmd->args[2]);
                    for (idx = 0; idx < temp; idx++) {
                        print_user(get_User(table, idx));
                    }
                }
                else if (!strncmp(cmd->args[1],"offset",6) && cmd->args[2])
                {
                    size_t temp=atoi(cmd->args[2]);
                    for (idx = temp; idx < table->len; idx++) {
                        print_user(get_User(table, idx));
                    }
                }
            }
    
    
    
   
    
    
   
    cmd->type = SELECT_CMD;

    return table->len;
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

