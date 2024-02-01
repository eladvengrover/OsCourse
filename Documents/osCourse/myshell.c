
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


int prepare(void) {
    return 0;
}



int process_arglist(int count, char** arglist) {
    switch (get_operation_type(count, arglist)) {
    case REG:
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
}

int handle_reg_op(int count, char** arglist) {

}

int handle_reg_bg_op(int count, char** arglist) {
    
}

int handle_pipe_op(int count, char** arglist) {

}

int handle_input_op(int count, char** arglist) {
    
}

int handle_output_op(int count, char** arglist) {
    
}

OperationType get_operation_type(int count, char **arglist) {
    if (count > 0 && strcmp(arglist[count - 1], AMPERSAND)) {
        return REG_BG;
    }
    if (count > 1 && strcmp(arglist[count - 2], LEFT_REDIRECT)) {
        return INPUT;
    }
    if (count > 1 && strcmp(arglist[count - 2], RIGHT_REDIRECT)) {
        return OUTPUT;
    }
    for (int i = 1; i < count; i++) {
        if (strcmp(arglist[i], PIPE)) {
            return SING_PIPE;
        }
    }
    return REG;
}






int finalize(void) {
    return 0;
}