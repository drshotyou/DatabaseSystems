#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "Command.h"
#include "SelectState.h"
//DO EVERYTHING HEREEEE
void field_state_handler(Command_t *cmd, size_t arg_idx) {
    //More segmentation faults w/o printing/getting here???
    cmd->cmd_args.sel_args.fields = NULL;
    cmd->cmd_args.sel_args.fields_len = 0;
    cmd->cmd_args.sel_args.limit = -1;
    cmd->cmd_args.sel_args.offset = -1;
    //printf("FSH 1");
    cmd->aggs.aggre=0;
    cmd->aggs.sumid=-1;
    cmd->aggs.sumage=-1;
    cmd->aggs.avgid=-1;
    cmd->aggs.avgage=-1;

    
    while(arg_idx < cmd->args_len) {
        if (!strncmp(cmd->args[arg_idx], "*", 1)) {
            //printf("printUsers 2");
            add_select_field(cmd, cmd->args[arg_idx]);
        } else if (!strncmp(cmd->args[arg_idx], "id", 2)) {
            add_select_field(cmd, cmd->args[arg_idx]);
        } else if (!strncmp(cmd->args[arg_idx], "name", 4)) {
            add_select_field(cmd, cmd->args[arg_idx]);
        } else if (!strncmp(cmd->args[arg_idx], "email", 5)) {
            add_select_field(cmd, cmd->args[arg_idx]);
        } else if (!strncmp(cmd->args[arg_idx], "age", 3)) {
            add_select_field(cmd, cmd->args[arg_idx]);
        } else if (!strncmp(cmd->args[arg_idx], "from", 4)) {
            table_state_handler(cmd, arg_idx+1);
            //printf("FSH from\n");
            //arg_idx++;
            return;
        } else if (!strncmp(cmd->args[arg_idx], "sum(id)", 7)) {
            cmd->aggs.sumid=1;
            cmd->aggs.aggre=1;
            add_select_field(cmd, cmd->args[arg_idx]);
        } else if (!strncmp(cmd->args[arg_idx], "sum(age)", 8)) {
            cmd->aggs.sumage=1;
            cmd->aggs.aggre=1;
            add_select_field(cmd, cmd->args[arg_idx]);
        } else if (!strncmp(cmd->args[arg_idx], "avg(id)", 7)) {
            cmd->aggs.avgid=1;
            cmd->aggs.aggre=1;
            add_select_field(cmd, cmd->args[arg_idx]);
        } else if (!strncmp(cmd->args[arg_idx], "avg(age)", 8)) {
            cmd->aggs.avgage=1;
            cmd->aggs.aggre=1;
            add_select_field(cmd, cmd->args[arg_idx]);
        } else if (!strncmp(cmd->args[arg_idx], "count", 5)) {
            cmd->aggs.aggre=1;
            add_select_field(cmd, cmd->args[arg_idx]);
        } else {
            cmd->type = UNRECOG_CMD;
            return;
        }
        arg_idx += 1;
    }
    cmd->type = UNRECOG_CMD;
    return;
}

void table_state_handler(Command_t *cmd, size_t arg_idx) {
    if (arg_idx < cmd->args_len && !strncmp(cmd->args[arg_idx], "table", 5)) {

        arg_idx++;
        if (arg_idx == cmd->args_len) {
            return;
        } else if (!strncmp(cmd->args[arg_idx], "offset", 6)) {
            offset_state_handler(cmd, arg_idx+1);
            return;
        } else if (!strncmp(cmd->args[arg_idx], "limit", 5)) {
            limit_state_handler(cmd, arg_idx+1);
            return;
        } else if (!strncmp(cmd->args[arg_idx],"where",5))
        {
            where_state_handler(cmd,arg_idx+1);
            //printf("TSH ret\n");
            return;
        }
    }
    cmd->type = UNRECOG_CMD;
    return;
}

void offset_state_handler(Command_t *cmd, size_t arg_idx) {
    if (arg_idx < cmd->args_len ) {
        cmd->cmd_args.sel_args.offset = atoi(cmd->args[arg_idx]);

        arg_idx++;

        if (arg_idx == cmd->args_len) {
            return;
        } else if (arg_idx < cmd->args_len
                && !strncmp(cmd->args[arg_idx], "limit", 5)) {

            limit_state_handler(cmd, arg_idx+1);
            return;
        }
    }
    cmd->type = UNRECOG_CMD;
    return;
}

void limit_state_handler(Command_t *cmd, size_t arg_idx) {
    if (arg_idx < cmd->args_len) {
        cmd->cmd_args.sel_args.limit = atoi(cmd->args[arg_idx]);

        arg_idx++;

        if (arg_idx == cmd->args_len) {
            return;
        } else if(arg_idx < cmd->args_len &&
         !strncmp(cmd->args[arg_idx],"offset",5)){ //make it work in both ways?
             offset_state_handler(cmd,arg_idx);
             return;
         }
    }
    cmd->type = UNRECOG_CMD;
    return;
}
/*Possible combinations
//try using an array for all possible conditions?
//switch only works for int vals...
//ONLY TWO CONDS AT MOST, NO NEED TO COMPLICATE YOURSELF
log     2 0==NULL 1==and 2==or
cond1   0
cond2   1
op1     3
op2     4
//cond combs 1 id, 2 age, 3 name, 4 email
1 1                      
1 2      2 2             
1 3      2 3     3 3     
1 4      2 4     3 4     4 4
//Op combs 1 = , 2 !=, 3 > ,4 <, 5 >=, 6 <=
//string
1 1      2 2
1 2
//Numeric
1 1      
1 2      2 2    
1 3      2 3    3 3     
1 4      2 4    3 4     4 4
1 5      2 5    3 5     4 5     5 5
1 6      2 6    3 6     4 6     5 6     6 6


id name
1 1             3 1     4 1     5 1     6 1 
1 2     2 2     3 2     4 2     5 2     6 2
name name
1 1 
1 2     2 2
*/
//same idea as select but for conditions here
void where_state_handler(Command_t *cmd, size_t arg_idx){
    cmd->conds.where=1;
    cmd->conds.logOp=0;
    cmd->conds.stringType_len=0;
    cmd->conds.numType_len=0;
    cmd->conds.condsStr=(char **)malloc(sizeof(char*)*5);
    cmd->conds.condsNum=(char **)malloc(sizeof(char*)*5);
    //printf("WSH BEGIN INI\n");
    for(int i=0;i<2;i++)
    {
        int op;
        int temp1 = cmd->conds.stringType_len;
        int temp2 = cmd->conds.numType_len;
        char *cond=cmd->args[arg_idx];
        char *compOp=cmd->args[arg_idx+1];
        char *cond1=cmd->args[arg_idx+2];
        
        if(!strncmp(cond,"name",4) || !strncmp(cond1,"name",4) || !strncmp(cond,"email",5) || !strncmp(cond1,"email",5))
        {
            //printf("WSH BEGIN 1\n");
            cmd->conds.condsStr[(2*temp1)]=cond;
            cmd->conds.condsStr[(2*temp1)+1]=cond1;

            if(!strncmp(compOp,"=",1))
            {
                op=1;
            } else if (!strncmp(compOp,"!=",2))
            {
                op=2;
            }
            cmd->conds.strCond[temp1] = op;
            cmd->conds.stringType_len++;
        } 
            else
        {
            //printf("WSH BEGIN 2\n");
            cmd->conds.condsNum[temp2]=cond;
            cmd->conds.intOp[temp2]=atoi(cond1);
            if(!strncmp(compOp,"=",1))
            {
                op=0;
            } else if(!strncmp(compOp,"!=",2))
            {
                op=1;
            } else if(!strncmp(compOp,">=",2))
            {
                op=4;
            } else if(!strncmp(compOp,"<=",2))
            {
                op=5; 
            } else if(!strncmp(compOp,">",1))
            {
                op=2;
            } else if(!strncmp(compOp,"<",1))
            {
                op=3;
            }
            cmd->conds.numCond[temp2]=op;
            cmd->conds.numType_len++;

        }
        //printf("WSH BEGIN 3 %d %d\n",arg_idx+3,cmd->args_len);
         if(arg_idx+3>=cmd->args_len)
        {
            //printf("WSH BEGIN 4\n");
            break;
        } else if(!strncmp(cmd->args[arg_idx+3],"and",3))
        {
            //printf("AND\n");
            cmd->conds.logOp=1;
        } else if(!strncmp(cmd->args[arg_idx+3],"or",2))
        {
            //printf("OR\n");
            cmd->conds.logOp=2;
        } else { 
            arg_idx+=3;
            //printf("WSH BEGIN 5\n");
            break;
        }
        //printf("WSH BEGIN 10\n");
        arg_idx +=4;
    }
    //printf("WSH BEGIN 6\n");
    if (arg_idx == cmd->args_len) {
        //printf("WSH BEGIN 7\n");
        return;
    } else if (!strncmp(cmd->args[arg_idx], "offset", 6)) {
       // printf("WSH BEGIN 8\n");
        offset_state_handler(cmd, arg_idx+1);
        return;
    } else if (!strncmp(cmd->args[arg_idx], "limit", 5)) {
        //printf("WSH BEGIN 9\n");
        limit_state_handler(cmd, arg_idx+1);
        return;
    }
}

void set_state_handler(Command_t *cmd, size_t arg_idx)
{
    int state=0;
    if (!strncmp(cmd->args[arg_idx], "name", 4) || !strncmp(cmd->args[arg_idx], "email", 5)) {
		state = cmd->updates.soi = 1;
    }
    cmd->updates.field = cmd->args[arg_idx];
    if(state==1)
    {
        cmd->updates.strs = cmd->args[arg_idx+2];
    }
    else
    {
        cmd->updates.ints=atoi(cmd->args[arg_idx+2]);
    }
    arg_idx+=3;
    if(arg_idx>=cmd->args_len)
    {
        return;
    } else if(!strncmp(cmd->args[arg_idx],"where",5))
    {
        where_state_handler(cmd,arg_idx+1);
        return;
    }
    
}