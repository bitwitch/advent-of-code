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

void part_one(Node **stacks, size_t stack_count, char *instructions, size_t instruction_count) {
    int i;
    Node *node;

    for (i=0; i<stack_count; ++i) {
        printf("stack %d:", i);
        node = stacks[i];
        while (node) {
            printf(" %c", node->data);    
            node = node->next;
        }
        printf("\n");
    }
}

void part_two(Node **stacks, size_t stack_count, char *instructions, size_t instruction_count) {
    printf("part_two: %d\n", 696969);
}

bool parse_input(uint8_t *file_data, size_t file_size, 
                 Node ***stacks, size_t *stack_count, 
                 char **instructions, size_t *instruction_count) 
{
    char *lines[256];
    char *line = NULL;
    uint32_t max_stack_height = 0;
    int i, j;

    //
    // parse stacks
    //
    printf("parse stacks\n");
    do {
        line = arb_chop_by_delimiter((char**)&file_data, "\n");

        if (strlen(line) > 2 && line[1] == '1') {
            *stack_count = line[strlen(line) - 1] - 48;
            break;
        }
        lines[max_stack_height] = line;
        printf("line: %s\n", line);
        ++max_stack_height;
    } while(*file_data != '\0');

    printf("stack count: %zu, max height: %u\n", *stack_count, max_stack_height);

    // allocate storge for the supply stacks
    Node **temp = calloc(*stack_count, sizeof(Node*));
    if (!temp) { perror("calloc"); exit(1); }
    *stacks = temp;

    for (i = max_stack_height; i > 0; --i) {
        line = lines[i];
        for (j=1; j<strlen(line); j+=4) {
            Node *node = malloc(sizeof(Node));
            if (!node) { perror("malloc"); exit(1); }

            int stack_index = (j-1)/4;
            assert(stack_index < *stack_count);
            node->data = line[j];
            node->next = (*stacks)[stack_index];

            (*stacks)[stack_index] = node;
        }
    }

    //
    // parse instructions
    //
    printf("parse instructions\n");
    do {
        line = arb_chop_by_delimiter((char**)&file_data, "\n");
        if (strcmp(line, "") == 0) continue; // skip empty lines
        printf("line: %s\n", line);
        *instruction_count = *instruction_count + 1;
    } while(*file_data != '\0');

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
    Node **stacks;
    size_t stack_count;

    char *instructions;
    size_t instruction_count;

    bool success = parse_input(file_data, file_size, &stacks, &stack_count, &instructions, &instruction_count);
    if (!success) { perror("parse_input"); exit(1); }

    part_one(stacks, stack_count, instructions, instruction_count);
    part_two(stacks, stack_count, instructions, instruction_count);

    return 0;
}

