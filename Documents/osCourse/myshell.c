#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

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
int get_pipe_index(int count, char **arglist);
void exec_command(char **arglist);
int wait_pid(int pid);



int prepare(void) {
    return 0;
}



int process_arglist(int count, char** arglist) {
    OperationType op = get_operation_type(count, arglist);
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
        perror("fork error!");
        return 0;
    }
    if (pid == 0) {
        // Child proccess
        exec_command(arglist);
    }
    // Parent proccess
    return wait_pid(pid);
}

int handle_reg_bg_op(int count, char** arglist) {
    int pid = fork();
    if (pid == -1) {
        perror("fork error!");
        return 0;
    }
    if (pid == 0) {
        // Child proccess
        arglist[count - 1] = NULL;
        exec_command(arglist);
    }
    return 1;
}

int handle_pipe_op(int count, char** arglist) {
    int pfds[2];
    int pipe_index = get_pipe_index(count, arglist);
    arglist[pipe_index] = NULL;
    if (pipe(pfds) == -1) {
        perror("pipe error!");
        return 0;
    }
    int pid = fork();
    if (pid == -1) {
        perror("fork error!");
        close(pfds[0]);
        close(pfds[1]);
        return 0;
    }
    if (pid == 0) {
        // Child proccess #1
        close(pfds[0]);
        if (dup2(pfds[1], STDOUT_FILENO) == -1) {
            perror("dup2 error!");
            exit(1);
        }
        close(pfds[1]);
        exec_command(arglist);
    }
    // Parent proccess
    int pid_2 = fork();
    if (pid_2 == -1) {
        perror("fork error!");
        close(pfds[0]);
        close(pfds[1]);
        return 0;
    }
    if (pid_2 == 0) {
        // Child proccess #2
        close(pfds[1]);
        if (dup2(pfds[0], STDIN_FILENO) == -1) {
            perror("dup2 error!");
            exit(1);
        }
        close(pfds[0]);
        exec_command(arglist + pipe_index + 1);
    }
    // Parent proccess
    close(pfds[0]);
    close(pfds[1]);
    // Waiting for children
    return ((wait_pid(pid) == 0) || (wait_pid(pid_2) == 0)) ? 0 : 1;
}

int handle_input_op(int count, char** arglist) {
    int fd = open(arglist[count - 1], O_RDONLY);
    arglist[count - 2] = NULL;
    if (fd == -1) {
        perror("file opening error!");
        return 0;
    }
    int pid = fork();
    if (pid == -1) {
        perror("fork error!");
        return 0;
    }
    if (pid == 0) {
        // Child proccess
        if (dup2(fd, STDIN_FILENO) == -1) {
            perror("dup2 error!");
            exit(1);
        }
        close(fd);
        exec_command(arglist);
    }
    // Parent proccess
    return wait_pid(pid);
}

int handle_output_op(int count, char** arglist) {
    int fd = open(arglist[count - 1], O_WRONLY | O_CREAT | O_TRUNC, 0777);
    arglist[count - 2] = NULL;
    if (fd == -1) {
        perror("file opening error!");
        return 0;
    }
    int pid = fork();
    if (pid == -1) {
        perror("fork error!");
        return 0;
    }
    if (pid == 0) {
        // Child proccess
        if (dup2(fd, STDOUT_FILENO) == -1) {
            perror("dup2 error!");
            exit(1);
        }
        close(fd);
        exec_command(arglist);
    }
    // Parent proccess
    return wait_pid(pid);
}

int get_pipe_index(int count, char **arglist) {
    int i;
    for (i = 0; i < count; i++) {
        if (strcmp(arglist[i], ((char[]) {PIPE, '\0'})) == 0) {
            return i;
        }
    }
    return -1;
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
    // TODO - add check for command validation
    return get_pipe_index(count, arglist) != -1 ? SING_PIPE : REG;
}

void exec_command(char **arglist) {
    if (execvp(arglist[0], arglist) == -1) {
        perror("execvp failed!");
        exit(1);
    }
}

int wait_pid(int pid) {
    if (waitpid(pid, NULL, 0) == -1 && errno != ECHILD && errno != EINTR) {
        perror("waitpid error!");
        return 0;
    }
    return 1;
}




int finalize(void) {
    return 0;
}