#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "../base/base_inc.h"
#include "../base/base_inc.c"

typedef struct Node Node;

struct Node {
    Node *next;
    char data;
};

typedef struct {
    U32 count, src, dest;
} Instruction;

void print_stacks(Node **stacks, size_t stack_count) {
    Node *node;
    size_t i;
    for (i=0; i<stack_count; ++i) {
        node = stacks[i];
        while (node) {
            printf("%c ", node->data);
            node = node->next;
        }
        printf("\n");

    }
    printf("\n");
}

void move_crate_from_to(Node **stacks, U32 src, U32 dest) {
    Node *mover = stacks[src];
    if (mover) {
        stacks[src] = mover->next;
        mover->next = stacks[dest];
        stacks[dest] = mover;
    }
}

Node *pop_crate(Node **stacks, U32 index) {
    Node *stack = stacks[index];
    if (!stack) return NULL;
    stacks[index] = stack->next;
    stack->next = NULL;
    return stack;
}

void part_one(Node **stacks, size_t stack_count, Instruction *instructions, size_t instruction_count) {
    size_t i, j;
    for (i=0; i<instruction_count; ++i)
        for (j=0; j < instructions[i].count; ++j)
            move_crate_from_to(stacks, instructions[i].src-1, instructions[i].dest-1);

    char result[256];
    j = 0;
    for (i=0; i<stack_count; ++i) {
        if (stacks[i]) {
            result[j++] = stacks[i]->data;
        }
    }
    result[j] = '\0';
    
    printf("part_one: %s\n", result);
}

void part_two(Node **stacks, size_t stack_count, Instruction *instructions, size_t instruction_count) {
    size_t i, j, src, dest;
    Node *node, *head;
    for (i=0; i<instruction_count; ++i) {
        src = instructions[i].src-1;
        dest = instructions[i].dest-1;
        head = stacks[src];

        node = stacks[src];
        for (j=1; j < instructions[i].count; ++j) {
            assert(node);
            node = node->next;
        }

        stacks[src] = node->next;
        node->next = stacks[dest];
        stacks[dest] = head;
    }

    char result[256];
    j = 0;
    for (i=0; i<stack_count; ++i) {
        if (stacks[i]) {
            result[j++] = stacks[i]->data;
        }
    }
    result[j] = '\0';
    printf("part_two: %s\n", result);
}

bool parse_input(uint8_t *file_data, size_t file_size, 
                 Node ***stacks, size_t *stack_count, 
                 Instruction **instructions, size_t *instruction_count) 
{
    (void)file_size;

    char *lines[256];
    char *line = NULL;
    uint32_t max_stack_height = 0;
    int i, j;

    //
    // parse stacks
    //
    do {
        line = arb_chop_by_delimiter((char**)&file_data, "\n");

        if (strlen(line) > 2 && line[1] == '1') {
            // NOTE:(shaw) watch out here, this assumes that the last number in this line
            // is the second character from the end (and also that it is single
            // digit)
            *stack_count = line[strlen(line) - 2] - 48;
            break;
        }
        lines[max_stack_height] = line;
        ++max_stack_height;
    } while(*file_data != '\0');

    // allocate storge for the supply stacks
    Node **temp = calloc(*stack_count, sizeof(Node*));
    if (!temp) { perror("calloc"); exit(1); }
    *stacks = temp;

    for (i = max_stack_height-1; i >= 0; --i) {
        line = lines[i];
        for (j=1; j<(int)strlen(line); j+=4) {
            if (line[j] == ' ') continue;
            Node *node = malloc(sizeof(Node));
            if (!node) { perror("malloc"); exit(1); }

            size_t stack_index = (j-1)/4;
            assert(stack_index < *stack_count);
            node->data = line[j];
            node->next = (*stacks)[stack_index];
            (*stacks)[stack_index] = node;
        }
    }

    //
    // parse instructions
    //
    Instruction *temp_inst;
    size_t temp_size = 0;

    do {
        line = arb_chop_by_delimiter((char**)&file_data, "\n");
        if (strcmp(line, "") == 0) continue; // skip empty lines

        if (*instruction_count + 1 > temp_size) {
            temp_inst = realloc(*instructions, (*instruction_count+1) * 2 * sizeof(Instruction));
            if (!temp_inst) { perror("realloc"); exit(1); }
            *instructions = temp_inst;
        }

        // gross, this is just getting a temp pointer to next slot in
        // instructions array
        temp_inst = &(*instructions)[*instruction_count];

        sscanf(line, "move %u from %u to %u", 
            &temp_inst->count, &temp_inst->src, &temp_inst->dest);

        *instruction_count = *instruction_count + 1;
    } while(*file_data != '\0');

    // resize instructions array to exact number of items
    temp_inst = realloc(*instructions, *instruction_count * sizeof(Instruction));
    if (!temp_inst) { perror("realloc"); exit(1); }
    *instructions = temp_inst;

    return true;
}

int main(int argc, char **argv) {

    if (argc < 2) {
        fprintf(stderr, "Must supply filepath to puzzle input\n");
        exit(1);
    }

    FILE *fp;
    uint8_t *file_data;
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

    // stacks is an array of linked lists, or in other words an array Node
    // pointers that are heads of a linked list
    Node **stacks = NULL;
    size_t stack_count = 0;

    Instruction *instructions = NULL;
    size_t instruction_count = 0;

    bool success = parse_input(file_data, file_size, &stacks, &stack_count, &instructions, &instruction_count);
    if (!success) { perror("parse_input"); exit(1); }

    /*part_one(stacks, stack_count, instructions, instruction_count);*/
    part_two(stacks, stack_count, instructions, instruction_count);

    return 0;
}

