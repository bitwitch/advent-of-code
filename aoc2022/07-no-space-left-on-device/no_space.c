#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "../base/base_inc.h"
#include "../base/base_inc.c"

#define GIF_WIDTH  1024
#define GIF_HEIGHT 1024
#define SMALL_DIR_SIZE     100000
#define SCROLL_BACK_LINES  8
#define TOTAL_DISK_SPACE          70000000
#define SPACE_REQUIRED_FOR_UPDATE 30000000
#define MAX_FILENAME_SIZE         256
#define PALETTE_COLORS 10
#define COLOR_GOLD 0x51d2e8
#define COLOR_GREY 0x232323

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

global Node *small_dirs[SCROLL_BACK_LINES];
global int small_dirs_count = 0; // the number in the small dirs buffer (capped by SCOLL_BACK_LINES)
global int total_small_dirs = 0; // the total number of small dirs found
global int total_small_dir_size = 0; // the total combined size of small dirs found

global int file_block_y = 610;
global int file_block_height = 300;

global U64 frame_count = 0;
global U64 frames_skip = 2;

U32 rgb_to_bgr(U32 color) {
    U32 r, g, b;
    b = (color >>  0) & 0xFF;
    g = (color >>  8) & 0xFF;
    r = (color >> 16) & 0xFF;
   return r | (g << 8) | (b << 16);
}

global U32 palette[PALETTE_COLORS] = {
    0x191200,
    0x735f00,
    0x96930a,
    0xbdd294,
    0xa6d8e9,
    0x009bee,
    0x0267ca,
    0x033ebb,
    0x1220ae,
    0x26229b
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
            // redundant work. the directory sizes could be cached if we want
            // to be efficient
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
    U64 sum = get_sum_under_max(root, SMALL_DIR_SIZE);
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

/*
 *                    /
 *    a         b           c                d
 * f g h e                                j d d k
 *        i




 [a b c j d d.ext k]
 / d



/
    a
        e
            i.txt
        f
        g
        h.lst
    b.txt
    c.dat
    d
        j
        d.log
        d.ext
        k
*/

void draw_file_block(int x, Node *node) {
    assert(node);
    local_persist int palette_index = 0;
    U64 file_size = node->type == NT_DIR ? directory_size(node) : node->size;
    int width = (file_size / (float)TOTAL_DISK_SPACE) * (GIF_WIDTH - 50);
    U32 color = palette[palette_index];

    if (node->type == NT_DIR)
        strokebox(25+x, file_block_y, width, file_block_height, 2, color);
    else
        drawbox(25+x, file_block_y, width, file_block_height, color);

    palette_index = (palette_index + 1) % PALETTE_COLORS;
}

void draw_file_path(int x, int y, int scale, int color, Node *node) {
    assert(node);
    char path[256];
    path[255] = '\0';
    char *start = path + 254;
    char *end = path + 255;
    Node *temp = node;
    size_t len;

    len = strlen(temp->name);
    start -= len;
    memcpy(start, temp->name, len);
    temp = temp->parent;

    while (temp) {
        if (0 != strcmp(temp->name, "/")) {
            --start;
            *start = '/';
        }
        len = strlen(temp->name);
        start -= len;
        memcpy(start, temp->name, len);
        temp = temp->parent;
    }

    // max display length is 47 characters
    if (end - start > 47) {
        start = end - 47;
        start[0] = '.';
        start[1] = '.';
        start[2] = '.';
    }

    drawstringf(x, y, scale, color, "%s", start);
}

void dfs(Node *root, int level, int x) {
    local_persist U64 used = 0; // disk spaced used

    if (root == NULL)
        return;

    Node *node;
    int i, block_width;
    U64 file_size;
    int root_x = x;

    // visit all children
    for (i=0; i<shlen(root->children); ++i) {
        node = root->children[i].value;
        dfs(node, level+1, x);

        file_size = node->type == NT_DIR ? directory_size(node) : node->size;
        block_width = (file_size / (float)TOTAL_DISK_SPACE) * (GIF_WIDTH - 50);
        x += block_width;
    }

    // visit node
    printf("level %d: %s\n", level, root->name);

    //
    // draw disk space visualization
    // 
    U64 root_size = root->type == NT_DIR ? directory_size(root) : root->size;

    if (root->type == NT_FILE)
        used += root_size;

    // update list of small dirs found
    if (root->type == NT_DIR && root_size <= SMALL_DIR_SIZE) {
        ++total_small_dirs;
        total_small_dir_size += root_size;
        if (small_dirs_count < SCROLL_BACK_LINES) {
            small_dirs[small_dirs_count++] = root;
        } else {
            memmove(small_dirs, small_dirs+1, (SCROLL_BACK_LINES - 1) * sizeof(small_dirs[0]));
            small_dirs[SCROLL_BACK_LINES-1] = root;
        }
    }

    // draw a black box to "clear" the text
    drawbox(0, 0, GIF_WIDTH, file_block_y, COLOR_GREY);

    // draw list of small directories found
    int line_height = 40;
    int line_y;
    for (i=0; i<small_dirs_count; ++i) {
        node = small_dirs[i];
        line_y = 25 + (i * line_height);
        if (small_dirs_count < SCROLL_BACK_LINES) {
            line_y += (SCROLL_BACK_LINES - small_dirs_count) * line_height;
        }
        drawstringf(280, line_y, 3, COLOR_GOLD,
            "%8s %d", node->name, directory_size(node));
    }
    drawstringf(210, 380, 3, COLOR_GOLD, "Directories under %6d", SMALL_DIR_SIZE);
    drawstringf(210, 420, 3, COLOR_GOLD, "          count: %7d", total_small_dirs); 
    drawstringf(210, 460, 3, COLOR_GOLD, "           size: %7d", total_small_dir_size);

    // draw current file name
    draw_file_path(30, 570, 2, 0xFFFFFF, root);
    // draw current file size
    drawstringf(810, 560, 3, 0xFFFFFF, "%8d", 
        root->type == NT_DIR ? root_size : root->size);
    
    draw_file_block(root_x, root);

    // draw used, free, and total space
    drawbox(0, file_block_y+file_block_height, 
        GIF_WIDTH, GIF_HEIGHT - (file_block_y+file_block_height), 
        COLOR_GREY); // clear the text
    drawstringf(680, 920, 2, 0xFFFFFF, "      used: %8d", used);
    drawstringf(680, 946, 2, 0xFFFFFF, "      free: %8d", TOTAL_DISK_SPACE - used);
    drawstringf(680, 972, 2, 0xFFFFFF, "disk space: %8d", TOTAL_DISK_SPACE);
    

    // flash the filename if its size is below max
    if (root->type == NT_DIR && root_size <= SMALL_DIR_SIZE) {
        draw_file_path(30, 570, 2, COLOR_GOLD, root);
        drawstringf(810, 560, 3, COLOR_GOLD, "%8d", 
            root->type == NT_DIR ? root_size : root->size);
        nextframe();
        nextframe();
    }

    ++frame_count;
    if (frame_count % frames_skip == 0)
        nextframe();
}


void visualization(Node *root) {
    drawbox(0, 0, GIF_WIDTH, GIF_HEIGHT, COLOR_GREY);
    drawbox(25, file_block_y, GIF_WIDTH-50, file_block_height, 0x000000);
    dfs(root, 0, 0);
}

#define MAX_QUEUE_SIZE 65536
void bfs(Node *root) {
    Node *node;
    Node *queue[MAX_QUEUE_SIZE] = {0};
    int q_front = 0;
    int q_rear = 0;
    int q_size = 0;

    int i;
    int level = 0;
    int items_left_in_level = 0;
    int x = 0;
    
    // push root
    queue[q_rear] = root;
    ++q_rear; ++q_size;

    drawbox(25, 512, GIF_WIDTH-50, file_block_height, 0xe3e3e3);
    while (q_size > 0) {
        // pop node
        node = queue[q_front];
        ++q_front; --q_size;

        assert(node);

        printf("level %d: %s\n", level, node->name);

        if (node->type == NT_DIR) {
            draw_file_block(x, node);
            x += (directory_size(node) / (float)TOTAL_DISK_SPACE) * (GIF_WIDTH - 50);
            nextframe();
        }

        for (i=0; i<shlen(node->children); ++i) {
            // push child
            queue[q_rear] = node->children[i].value;
            ++q_rear; ++q_size;
        }

        --items_left_in_level;
        if (items_left_in_level <= 0) {
            ++level;
            x = 0;
            items_left_in_level = q_size;
        }
    }
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
    
    setupgif(0, 1, "no_space.gif");

    Node *root = parse_filesystem(file_data, file_size);
    part_one(root);
    part_two(root);
    visualization(root);

    endgif();

    return 0;
}
