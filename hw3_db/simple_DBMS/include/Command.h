#ifndef COMMAND_H
#define COMMAND_H

enum { 
    UNRECOG_CMD,
    BUILT_IN_CMD,
    QUERY_CMD,
};

enum {
    INSERT_CMD = 100,
    SELECT_CMD,
};

typedef struct {
    char name[256];
    int len;
    unsigned char type;
} CMD_t;

extern CMD_t cmd_list[];
typedef struct aggregateFunctions {
    int aggre;
    int sumid;
    int avgid;
    int sumage;
    int avgage;
    int idTotal;
    int ageTotal;
    double idCalc;
    double ageCalc;
    int countTotal;
} aggregate_t;

typedef struct SelectArgs {
    char **fields;
    size_t fields_len;
    int offset;
    int limit;
} SelectArgs_t;

typedef struct conditions {
    int where;
    int logOp;
    int stringType_len;
    int numType_len;
    int intOp[2];
    char ** condsStr;
    char ** condsNum;
    int numCond[2];
    int strCond[2];
} conditions_t;

typedef union {
    SelectArgs_t sel_args;
} CmdArg_t;

typedef struct updates {
    int soi;
    int ints;
    char* field;
    char* strs;
}updates_t;

typedef struct Command {
    unsigned char type;
    char **args;
    size_t args_len;
    size_t args_cap;
    CmdArg_t cmd_args;
    conditions_t conds;
    aggregate_t aggs;
    updates_t updates;

} Command_t;

Command_t* new_Command();
int add_Arg(Command_t *cmd, const char *arg);
int add_select_field(Command_t *cmd, const char *argument);
void cleanup_Command(Command_t *cmd);

#endif
