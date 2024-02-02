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
    OUTPUT = 5,
    NOT_VALID = -1
} OperationType;

int operation_handler(int count, char** arglist, OperationType op);
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

    return operation_handler(count, arglist, op);
}

int operation_handler(int count, char** arglist, OperationType op) {
    switch (op) {
        case REG:
            return handle_reg_op(count, arglist);
        case REG_BG:
            return handle_reg_bg_op(count, arglist);
        case SING_PIPE:
            return handle_pipe_op(count, arglist);
        case INPUT:
            return handle_input_op(count, arglist);
        case OUTPUT:
            return handle_output_op(count, arglist);
        default:
            return 0;
    }
}

int handle_reg_op(int count, char** arglist) {
    int pid = fork();
    if (pid == -1) {
        return 0;
    }
    if (pid == 0) {
        // Child proccess
        if (execvp(arglist[0], arglist) == -1) {
        perror("execvp failed!");
        exit(1);
    }
    } else {
        // Parent proccess
        int status;
        if (waitpid(pid, &status, 0) == -1) {
            return 0;
        }
        return 1;
    }


    
    return 1;
}

int handle_reg_bg_op(int count, char** arglist) {
    int pid = fork();
    if (pid == -1) {
        return 0;
    }
    if (pid == 0) {
        // Child proccess
        free((char*) arglist[count]);
        arglist[count - 1] = NULL;
        
        if (execvp(arglist[0], arglist) == -1) {
            perror("execvp failed!");
            exit(1);
        }
    }
    return 1;
}

int handle_pipe_op(int count, char** arglist) {
    int pfds[2];
    if (pipe(pfds) == -1) {
        return -1;
    }
    int pid = fork();
    if (pid == -1) {
        return -1;
    }
    if (pid == 0) {
        // Child proccess
        close(pfds[0]);
    } else {
        // Parent proccess
    
    }

    return 1;
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
    // TODO - add check for command validation
    return REG;
}






int finalize(void) {
    return 0;
}