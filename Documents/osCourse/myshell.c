#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <string.h>

typedef enum {
    AMPERSAND = '&',
    PIPE = '|',
    LEFT_REDIRECT = '<',
    RIGHT_REDIRECT = '>',
} Symbols;

typedef enum {
    REG = 1,
    REG_BG = 2,
    SING_PIPE = 3,
    INPUT = 4,
    OUTPUT = 5
} OperationType;

int child_handler(int count, char** arglist, OperationType op);
int handle_reg_op(int count, char** arglist);
int handle_reg_bg_op(int count, char** arglist);
int handle_pipe_op(int count, char** arglist);
int handle_input_op(int count, char** arglist);
int handle_output_op(int count, char** arglist);
OperationType get_operation_type(int count, char **arglist);



int prepare(void) {
    return 0;
}



int process_arglist(int count, char** arglist) {
    OperationType op = get_operation_type(count, arglist);
    printf("op: %d\n", op);
    int pid = fork();
    if (pid == -1) {
        return -1;
    }
    if (pid == 0) {
        // Child proccess
        return child_handler(count, arglist, op);
    } else {
        // Parent proccess
        int status, child_pid;
        while (op != REG_BG && (child_pid = wait(&status) > 0)) { }

        return 1;
    }
    
}

int child_handler(int count, char** arglist, OperationType op) {
    switch (op) {
        case REG:
            return handle_reg_op(count, arglist);
            break;
        case REG_BG:
            break;
        case SING_PIPE:
            break;
        case INPUT:
            break;
        case OUTPUT:
            break;
        default:
            break;
    }
    return 0;
}

int handle_reg_op(int count, char** arglist) {
    if (execvp(arglist[0], arglist) == -1) {
        perror("execvp failed!");
        exit(1);
    }
    return 1;
}

int handle_reg_bg_op(int count, char** arglist) {
    return 0;
}

int handle_pipe_op(int count, char** arglist) {
    return 0;
}

int handle_input_op(int count, char** arglist) {
    return 0;
}

int handle_output_op(int count, char** arglist) {
    return 0;
}

OperationType get_operation_type(int count, char **arglist) {
    if (count > 0 && strcmp(arglist[count - 1], ((char[]) {AMPERSAND, '\0'})) == 0) {
        return REG_BG;
    }
    if (count > 1 && strcmp(arglist[count - 2], ((char[]) {LEFT_REDIRECT, '\0'})) == 0) {
        return INPUT;
    }
    if (count > 1 && strcmp(arglist[count - 2], ((char[]) {RIGHT_REDIRECT, '\0'})) == 0) {
        return OUTPUT;
    }
    for (int i = 1; i < count; i++) {
        if (strcmp(arglist[i], ((char[]) {PIPE, '\0'})) == 0) {
            return SING_PIPE;
        }
    }
    return REG;
}






int finalize(void) {
    return 0;
}