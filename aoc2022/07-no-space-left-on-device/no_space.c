#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "../base/base_inc.h"
#include "../base/base_inc.c"

typedef enum {
    NT_DIR,
    NT_FILE
} Node_Type;

typedef struct Node Node;

struct Node {
    Node_Type type;
    char *name;
    U64 size;
    Node *children;
};


void part_one(uint8_t *input) {
    U64 max_size = 100000;
    U64 sum = 0, dir_size = 0;
    char *line;
    
    /*Node *wd; // working directory*/

    do {
        line = arb_chop_by_delimiter((char**)&input, "\n");

        if (line[0] == '$') {
            // if its a cd command, change dir
            if (strstr(line, "cd")) {
                char buf[256] = {0};
                sscanf(line, "$ cd %s", buf);
            
                if (dir_size <= max_size)
                    sum += dir_size;

                dir_size = 0;

                /*wd = change_dir(buf);*/
            } else if (strstr(line, "ls")) {
                // if its an ls command, start parsing dirs and files
            } else {
                assert(0 && "unrecognized command");
            }
        } else {
            // output of a command 


            // assume command was ls
            //
            //
            // if its a dir, add it to tree
            if (strstr(line, "dir")) {
                /*char buf[256] = {0};*/
                /*sscanf(line, "dir %s", &buf);*/
                /*add_dir(buf);*/
            } else {
                // this is a file
                char buf[256] = {0};
                U64 n;
                sscanf(line, "%llu %s", &n, buf);
                dir_size += n;
            }
        }

        printf("line: %s\n", line);
    } while (*input != '\0');

    // go through each line

    printf("part_one: %llu\n", sum);;
}

void part_two(uint8_t *input) {
    printf("part_two: %d\n", 696969);
}

int main(int argc, char **argv) {

    if (argc < 2) {
        fprintf(stderr, "Must supply filepath to puzzle input\n");
        exit(1);
    }

    FILE *fp;
    uint8_t *file_data, *file_copy;
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

    file_copy = malloc(file_size);
    if (!file_copy) { perror("malloc"); exit(1); }
    memcpy(file_copy, file_data, file_size);

    part_one(file_copy);
    part_two(file_data);

    return 0;
}
