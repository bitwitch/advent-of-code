#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "../base/base_inc.h"
#include "../base/base_inc.c"

#define TOTAL_DISK_SPACE          70000000
#define SPACE_REQUIRED_FOR_UPDATE 30000000
#define MAX_FILENAME_SIZE         256

typedef enum {
    NT_DIR,
    NT_FILE
} Node_Type;

typedef struct Node Node;

struct Node {
    Node_Type type;
    char *name;
    U64 size;
    Node *parent;
    struct { char *key; Node *value; } *children; // hashmap of children 
};

bool file_in_dir(Node *wd, char *filename) {
    assert(wd);
    return (shget(wd->children, filename) != NULL);
}

Node *make_dir(Node *wd, char *dirname) {
    Node *dir = NULL;

    if (wd)
        dir = shget(wd->children, dirname);

    // TODO(shaw): check if node is type dir

    if (!dir) {
        dir = malloc(sizeof(Node));
        if (!dir) { perror("malloc"); exit(1); }

        dir->type = NT_DIR;
        dir->name = strndup(dirname, MAX_FILENAME_SIZE);
        dir->size = 0;
        dir->children = NULL;
        if (wd) {
            shput(wd->children, dir->name, dir);
            dir->parent = wd;
        }
    }

    return dir;
}


Node *make_file(Node *wd, char *filename, U64 filesize) {
    Node *file = NULL;
    if (wd)
        file = shget(wd->children, filename);

    // TODO(shaw): make sure node is type file

    if (!file) {
        file = malloc(sizeof(Node));
        if (!file) { perror("malloc"); exit(1); }

        file->type = NT_FILE;
        file->name = strndup(filename, MAX_FILENAME_SIZE);
        file->size = filesize;
        file->children = NULL;

        assert(wd);
        shput(wd->children, file->name, file);
        file->parent = wd;
    } else {
        fprintf(stderr, "make_file: %s already exists, aborting without overwriting file\n", filename);
    }
    return file;
}


Node *change_dir(Node *wd, char *dirname) {
    assert(wd);
    Node *dir = shget(wd->children, dirname);
    if (!dir) {
        fprintf(stderr, "change_dir: %s not found\n", dirname);
        return NULL;
    }
    if (dir->type != NT_DIR) {
        fprintf(stderr, "change_dir: %s is not a directory\n", dirname);
        return NULL;
    }
    return dir;
}

U64 directory_size(Node *dir) {
    U64 sum = 0;
    int i;
    Node *child;

    if (dir == NULL) return 0;

    for (i=0; i < shlen(dir->children); ++i) {
        child = dir->children[i].value;
        if (child->type == NT_FILE)
            sum += child->size;
        else
            sum += directory_size(child);
    }
    return sum;
}

U64 get_sum_under_max(Node *dir, U64 max) {
    U64 result = 0;
    U64 dir_size = 0;
    int i;
    Node *child;

    if (dir == NULL) return 0;

    dir_size = directory_size(dir);
    if (dir_size <= max)
        result += dir_size;

    for (i=0; i < shlen(dir->children); ++i) {
        child = dir->children[i].value;
        if (child->type == NT_DIR)
            result += get_sum_under_max(child, max);
    }
    return result;
}

Node *parse_filesystem(uint8_t *input, size_t input_size) {
    (void)input_size;
    char *line;
    Node *node = NULL, 
         *root = NULL,
         *wd = NULL; // working directory

    do {
        line = arb_chop_by_delimiter((char**)&input, "\n");

        if (line[0] == '$') {

            // cd command
            if (strstr(line, "cd")) {
                char dirname[MAX_FILENAME_SIZE] = {0};
                sscanf(line, "$ cd %s", dirname);

                // backup one directory 
                if (0 == strncmp(dirname, "..", 2)) {
                    if (wd && wd->parent)
                        wd = wd->parent;
                    continue;
                } 
            
                // if dir does not exist, create it
                node = make_dir(wd, dirname);

                // on first cd command, set working directory
                if (!wd) {
                    root = node;
                    wd = root;
                } 
                else
                    wd = change_dir(wd, dirname);

            // ls command
            } else if (strstr(line, "ls")) {
                // start parsing dirs and files
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
                char dirname[MAX_FILENAME_SIZE] = {0};
                sscanf(line, "dir %s", dirname);
                make_dir(wd, dirname);
            } else {
                // this is a file
                char filename[MAX_FILENAME_SIZE] = {0};
                U64 filesize;
                sscanf(line, "%lu %s", &filesize, filename);
                make_file(wd, filename, filesize);
            }
        }
    } while (*input != '\0');

    return root;
}

Node *get_smallest_dir_greater_than(Node *dir, U64 min_size) {
    Node *result = NULL;
    U64 dir_size = 0;
    int i;
    Node *child, *temp;
    U64 cur_min = UINT64_MAX;

    if (dir == NULL) return 0;

    dir_size = directory_size(dir);
    if (dir_size >= min_size && dir_size < cur_min) {
        cur_min = dir_size;
        result = dir;
    }

    for (i=0; i < shlen(dir->children); ++i) {
        child = dir->children[i].value;
        if (child->type == NT_DIR) {
            temp = get_smallest_dir_greater_than(child, min_size);
            // NOTE(shaw): we are calling directory_size all over, doing
            // redundant work, being inefficient
            dir_size = directory_size(temp);
            if (dir_size >= min_size && dir_size < cur_min) {
                cur_min = dir_size;
                result = temp;
            }
        }
    }

    return result;
}

void part_one(Node *root) {
    U64 max_size = 100000;
    U64 sum = get_sum_under_max(root, max_size);
    printf("part_one: %lu\n", sum);;
}

void part_two(Node *root) {
    U64 space_to_delete;
    U64 result = 0;
    U64 total_unused_space = TOTAL_DISK_SPACE - directory_size(root);
    Node *dir;

    if (total_unused_space < SPACE_REQUIRED_FOR_UPDATE) {
        space_to_delete = SPACE_REQUIRED_FOR_UPDATE - total_unused_space;
        dir = get_smallest_dir_greater_than(root, space_to_delete);
        result = directory_size(dir);
    }
    printf("part_two: %lu\n", result);
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

    Node *root = parse_filesystem(file_data, file_size);

    part_one(root);
    part_two(root);

    return 0;
}
