#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "../../base/base_inc.h"
#include "../../base/base_inc.c"

#define CRT_ROWS 6
#define CRT_COLS 40

typedef enum {
    INST_TYPE_NOOP = 0,
    INST_TYPE_ADDX,
    INST_TYPE_COUNT
} Instruction_Type;

typedef struct {
    Instruction_Type type;
    U32 cycles;
    S64 operand;
} Instruction;

void part_one(Instruction *instructions) {
    U64 inst_cycles = 0, cycles = 0;
    S64 x = 1;
    int i = 0;
    Instruction instruction;
    int instruction_count = arrlen(instructions);
    U64 check_cycle = 20;
    U64 check_cycle_interval = 40;
    U64 signal_strength = 0;

    while (1) {
        if (cycles == check_cycle) {
            signal_strength += cycles * x;
            check_cycle += check_cycle_interval;
        }

        if (inst_cycles <= 0) {
            // execute instruction
            if (instruction.type == INST_TYPE_ADDX)
                x += instruction.operand;

            // read next instruction
            if (i >= instruction_count) break;
            instruction = instructions[i++];
            inst_cycles = instruction.cycles;
        }

        ++cycles;
        --inst_cycles;
    }

    printf("part_one: %llu\n", signal_strength);
}

void part_two(Instruction *instructions) {
    char screen[CRT_ROWS][CRT_COLS];
    U64 inst_cycles = 0, cycles = 0, scanline = 0;
    S64 x = 1;
    int i = 0;
    Instruction instruction;
    int instruction_count = arrlen(instructions);
    U64 check_cycle = 20;
    U64 check_cycle_interval = 40;
    U64 signal_strength = 0;

    while (1) {
        if (cycles == check_cycle) {
            signal_strength += cycles * x;
            check_cycle += check_cycle_interval;
        }

        if (inst_cycles <= 0) {
            // execute instruction
            if (instruction.type == INST_TYPE_ADDX)
                x += instruction.operand;

            // read next instruction
            if (i >= instruction_count) break;
            instruction = instructions[i++];
            inst_cycles = instruction.cycles;
        }

        // draw pixel
        int col = cycles % CRT_COLS;
        char pixel = ' ';
        if (col >= x-1 && col <= x+1)
            pixel = '#';
        screen[scanline][col] = pixel;

        ++cycles;
        --inst_cycles;
        if (cycles % CRT_COLS == 0) ++scanline;
    }

    printf("part_two:\n"); 
    for (i=0; i<CRT_ROWS; ++i)
        printf("%.*s\n", CRT_COLS, screen[i]);
}

Instruction *parse_instructions(U8 *input, size_t input_size) {
    (void)input_size;
    Instruction *instructions = NULL;
    char *line;
    char name[5];

    do {
        Instruction instruction = {0};
        line = arb_chop_by_delimiter((char**)&input, "\n");
        sscanf(line, "%s %lld", name, &instruction.operand);
        if (0 == strcmp(name, "addx")) {
            instruction.type = INST_TYPE_ADDX;
            instruction.cycles = 2;
        } else {
            instruction.type = INST_TYPE_NOOP;
            instruction.cycles = 1;
        }
        arrput(instructions, instruction);
    } while (*input != '\0');

    return instructions;
}

int main(int argc, char **argv) {

    if (argc < 2) {
        fprintf(stderr, "Must supply filepath to puzzle input\n");
        exit(1);
    }

    FILE *fp;
    U8 *file_data;
    size_t file_size;

    fp = fopen(argv[1], "r");
    if (!fp) { perror("fopen"); exit(1); }

    int ok = arb_read_entire_file(fp, &file_data, &file_size);
    if (ok != ARB_READ_ENTIRE_FILE_OK) {
        fprintf(stderr, "Error: arb_read_entire_file()\n");
        fclose(fp);
        exit(1);
    }

    fclose(fp);

    Instruction *instructions = parse_instructions(file_data, file_size);

    part_one(instructions);
    part_two(instructions);

    return 0;
}
