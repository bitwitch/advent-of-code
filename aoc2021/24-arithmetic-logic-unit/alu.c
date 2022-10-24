#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <stdbool.h>

enum Operands {
    OP_INP,
    OP_ADD,
    OP_ADD_IMM,
    OP_MUL,
    OP_MUL_IMM,
    OP_DIV,
    OP_DIV_IMM,
    OP_MOD,
    OP_MOD_IMM,
    OP_EQL,
    OP_EQL_IMM,
    OP_COUNT
};

enum Register_Names {
    REG_X,
    REG_Y,
    REG_Z,
    REG_W,
    REG_COUNT
};

typedef struct {
    uint64_t regs[4];
    int input_index;
} Alu;

typedef struct {
    char name[4];
    uint8_t opcode;
    int operands[2];
} Instruction;



/*
 *
 *
    inp a - Read an input value and write it to variable a.
    add a b - Add the value of a to the value of b, then store the result in variable a.
    mul a b - Multiply the value of a by the value of b, then store the result in variable a.
    div a b - Divide the value of a by the value of b, truncate the result to an integer, then store the result in variable a. (Here, "truncate" means to round the value toward zero.)
    mod a b - Divide the value of a by the value of b, then store the remainder in variable a. (This is also called the modulo operation.)
    eql a b - If the value of a and b are equal, then store the value 1 in variable a. Otherwise, store the value 0 in variable a.

 * */

#define MAX_INPUT_SIZE 256
#define MAX_INSTRUCTIONS 1024


Instruction parse_line(Alu *alu, char *line, size_t line_size) {
    assert(line_size >= 5);

    Instruction instruction = {0};
    strncpy(instruction.name, line, 3);

    int dest_reg;
    switch (line[4]) {
        case 'x': dest_reg = REG_X; break;
        case 'y': dest_reg = REG_Y; break;
        case 'z': dest_reg = REG_Z; break;
        case 'w': dest_reg = REG_W; break;
        default: break;
    }
    instruction.operands[0] = dest_reg;

    if (0 == strcmp(instruction.name, "inp")) {
        instruction.opcode = OP_INP;
    } else {
        assert(line_size >= 7);
        char b = line[6];
        int value;
        switch (b) {
            case 'x': value = REG_X; break;
            case 'y': value = REG_Y; break;
            case 'z': value = REG_Z; break;
            case 'w': value = REG_W; break;
            default:  value = atoi(&line[6]); break;
        }
        instruction.operands[1] = value;

        if (0 == strcmp(instruction.name, "add")) {
            if (b >= 'a' && b <= 'z')
                instruction.opcode = OP_ADD;
            else
                instruction.opcode = OP_ADD_IMM;
        } else if (0 == strcmp(instruction.name, "mul")) {
            if (b >= 'a' && b <= 'z')
                instruction.opcode = OP_MUL;
            else
                instruction.opcode = OP_MUL_IMM;
        } else if (0 == strcmp(instruction.name, "div")) {
            if (b >= 'a' && b <= 'z')
                instruction.opcode = OP_DIV;
            else
                instruction.opcode = OP_DIV_IMM;
        } else if (0 == strcmp(instruction.name, "mod")) {
            if (b >= 'a' && b <= 'z')
                instruction.opcode = OP_MOD;
            else
                instruction.opcode = OP_MOD_IMM;
        } else if (0 == strcmp(instruction.name, "eql")) {
            if (b >= 'a' && b <= 'z')
                instruction.opcode = OP_EQL;
            else
                instruction.opcode = OP_EQL_IMM;
        }
    }

    return instruction;
}


void execute(Alu *alu, Instruction instruction, char *input) {
    /*printf("executing: %s %d %d\n", instruction.name, instruction.operands[0], instruction.operands[1]);*/
    switch(instruction.opcode) {
        case OP_INP:
        {
            assert(alu->input_index < MAX_INPUT_SIZE);
            int reg = instruction.operands[0];
            const char *digit = &input[alu->input_index++];
            alu->regs[reg] = atoi(digit);
            break;
        }
        case OP_ADD:
        {
            int op0 = instruction.operands[0];
            int op1 = instruction.operands[1];
            alu->regs[op0] = alu->regs[op0] + alu->regs[op1];
            break;
        }
        case OP_ADD_IMM:
        {
            int op0 = instruction.operands[0];
            int op1 = instruction.operands[1];
            alu->regs[op0] = alu->regs[op0] + op1;
            break;
        }
        case OP_MUL:
        {
            int op0 = instruction.operands[0];
            int op1 = instruction.operands[1];
            alu->regs[op0] = alu->regs[op0] * alu->regs[op1];
            break;
        }
        case OP_MUL_IMM:
        {
            int op0 = instruction.operands[0];
            int op1 = instruction.operands[1];
            alu->regs[op0] = alu->regs[op0] * op1;
            break;
        }
        case OP_DIV:
        {
            int op0 = instruction.operands[0];
            int op1 = instruction.operands[1];
            alu->regs[op0] = alu->regs[op0] / alu->regs[op1];
            break;
        }
        case OP_DIV_IMM:
        {
            int op0 = instruction.operands[0];
            int op1 = instruction.operands[1];
            alu->regs[op0] = alu->regs[op0] / op1;
            break;
        }
        case OP_MOD:
        {
            int op0 = instruction.operands[0];
            int op1 = instruction.operands[1];
            alu->regs[op0] = alu->regs[op0] % alu->regs[op1];
            break;
        }
        case OP_MOD_IMM:
        {
            int op0 = instruction.operands[0];
            int op1 = instruction.operands[1];
            alu->regs[op0] = alu->regs[op0] % op1;
            break;
        }
        case OP_EQL:
        {
            int op0 = instruction.operands[0];
            int op1 = instruction.operands[1];
            alu->regs[op0] = alu->regs[op0] == alu->regs[op1];
            break;
        }
        case OP_EQL_IMM:
        {
            int op0 = instruction.operands[0];
            int op1 = instruction.operands[1];
            alu->regs[op0] = alu->regs[op0] == op1;
            break;
        }
        default:
            fprintf(stderr, "Error: Unknown instruction: %s\n", instruction.name);
            exit(1);
            break;
    }
}

bool check_input(char *input, Instruction *instructions, int instruction_count) {
    Alu alu = {0};
    for (int i=0; i<instruction_count; ++i) {
        Instruction instruction = instructions[i];
        execute(&alu, instruction, input);
    }
    return alu.regs[REG_Z] == 0;
}

int main(int argc, char **argv) {
    FILE *fp = fopen(argv[1], "r");
    if (!fp) {
        fprintf(stderr, "Error: %s\n", strerror(errno));
    }

    Alu alu = {0};
    Instruction instructions[MAX_INSTRUCTIONS];
    int instruction_count = 0;
    char *line = NULL;
    size_t line_size = 0;
    size_t bytes_read;

    while ((bytes_read = getline(&line, &line_size, fp) > 0)) {
        instructions[instruction_count++] = parse_line(&alu, line, line_size);
    }

    fclose(fp);

    char model_number[15] = {0};
    for (uint64_t i=99999999999999; i>=0; --i) {
        snprintf(model_number, 15, "%14lu", i);
        bool valid = check_input(model_number, instructions, instruction_count);
        if (valid) {
            printf("FOUND VALID MODEL NUMBER: %s\n", model_number);
        }
    }

    return 0;
}
