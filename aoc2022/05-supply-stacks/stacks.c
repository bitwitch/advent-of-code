#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "../base/base_inc.h"
#include "../base/base_inc.c"

#define GIF_SIZE     1024
#define SHIP_WIDTH   600
#define SHIP_HEIGHT  150
#define SHIP_START_Y (SHIP_HEIGHT + 150)
#define WAVE_SPEED   0.012
#define WAVE_AMP     10

typedef struct Node Node;

struct Node {
    Node *next;
    char data;
};

typedef struct {
    U32 count, src, dest;
} Instruction;

global U32 frames = 0;


void render_frame(void) {
    nextframe(0);
}

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






U32 palette[26] = {
    /*0xcd4a4a, 0xcc6666, 0xbc5d58, 0xff5349, */
    /*0xfd5e53, 0xfd7c6e, 0xfdbcb4, 0xff6e4a,*/
    /*0xffa089, 0xea7e5d, 0xb4674d, 0xa5694f,*/
    /*0x9f8170, 0xcd9575, 0xefcdb8, 0xd68a59,*/
    /*0xdeaa88, 0xfaa76c, 0xffcfab, 0xffbd88,*/
    /*0xffcf48, 0xfce883, 0xf0e891, 0xeceabe,*/
    /*0xc5e384, 0xb2ec5d*/

    0xffa134, 0x007a33, 0x1232cd, 0xffa600,
    0x007a27, 0xf87400, 0x5edb3b, 0xff6f00,
    0x840000, 0xff9500, 0x000079, 0xff6f00,
    0xff7b00, 0xff4b33, 0xffffff, 0xff6f00,
    0x007a27, 0x890000, 0xff6700, 0xd05700,
    0x840000, 0x007900, 0xdd007d, 0xfc5a00,
    0xff4ccc, 0x2eebde
};

U32 rgb_to_bgr(U32 color) {
    U32 r, g, b;
    b = (color >>  0) & 0xFF;
    g = (color >>  8) & 0xFF;
    r = (color >> 16) & 0xFF;
   return r | (g << 8) | (b << 16);
}

void draw_stacks(Node **stacks, size_t stack_count) {
    Node *node;
    size_t i, col;
    int pad = 3;
    int width = 40;
    int height = 15;

    float wave_offset = WAVE_AMP * sin((float)frames * WAVE_SPEED);

    // we want to draw the stack in reverse order, but since its a linked list
    // we gotta get the height first so we know where to draw each crate
    // this is dumb af but whatever
    int heights[stack_count];
    int h;
    for (i=0; i<stack_count; ++i) {
        h = 0;
        node = stacks[i];
        while (node) {
            ++h;
            node = node->next;
        }
        heights[i] = h;
    }

    for (i=0; i<stack_count; ++i) {
        h = heights[i];
        node = stacks[i];
        col = 0;
        while (node) {
            drawbox(i*(width+pad) + GIF_SIZE/4 - ((width+pad)*stack_count)/2, 
                    GIF_SIZE - SHIP_START_Y - ((h - col)*height) + wave_offset, 
                    width, height, 
                    rgb_to_bgr(palette[node->data - 65]));
                    /*palette[node->data - 65]);*/
            ++col;
            node = node->next;
        }
    }
}

void draw_elf(int x, int y, int width, int height) {
    // green box torso
    drawbox(x - width/4, 
            y + height/4, 
            width/2, height/2, 
            rgb_to_bgr(0x6caa00));
 
    //circle head
    drawcircle(x, y, width/2, rgb_to_bgr(0xfdc297));
    // eyes
    drawcircle(x-6, y, 3, rgb_to_bgr(0x7d4011));
    drawcircle(x+6, y, 3, rgb_to_bgr(0x7d4011));

    // cigarette
    drawbox(x - 20, y+height/8 + 1,
            20, 4, 
            0xffffff);
    drawcircle(x - 20 - 1, y+height/8 + 1, 1, 0x0000ff);


    // line, red hat brim
    drawbox(x - width/2, 
            y - height/4, 
            width, height/10, 
            rgb_to_bgr(0xe6342e));
     
    // green triangle hat
    drawtri(x - width/2, y - height/4, 
            x, y - height, 
            x + width/2, y - height/4, 
            rgb_to_bgr(0x6caa00));

    // brown box shoes
     drawbox(x - width/4 - 2, 
            y + height/4 + height/2, 
            width/2 + 4, height/10 + 1, 
            rgb_to_bgr(0x7d4011));
}


void draw_bg(void) {
    U32 sky = rgb_to_bgr(0xd9e3f6);
    U32 deep = rgb_to_bgr(0x094d4b);
    U32 crane = rgb_to_bgr(0xb9362d);
    U32 dock = rgb_to_bgr(0x543b2c);

    // sky
    drawbox(0, 0, GIF_SIZE, GIF_SIZE, sky);

    // water
    drawbox(0, GIF_SIZE-200, GIF_SIZE, 200, deep);

    // dock
    /*drawbox(GIF_SIZE-400, GIF_SIZE-180, 400, 200, dock);*/
    drawtri(GIF_SIZE-400, GIF_SIZE-180,
            GIF_SIZE, GIF_SIZE-180,
            GIF_SIZE, 3*GIF_SIZE, dock);

    // crane
    drawbox(GIF_SIZE-325, 120, 40, GIF_SIZE-160, crane); // left leg
    drawbox(GIF_SIZE-70, 120, 40, GIF_SIZE-160, crane); // right leg
    drawbox(GIF_SIZE/4, 220, 760, 50, crane); // horizontal beam
    drawline(GIF_SIZE-325, 120, GIF_SIZE-70, 120, 4, crane); // cable
    drawline( GIF_SIZE/4, 220, GIF_SIZE-325, 120, 4, crane); // cable

    // elf
    int elf_w = 30, elf_h = 60;
    draw_elf(GIF_SIZE-150, 170, elf_w, elf_h);

}

void draw_ship(void) {
    local_persist U32 x = GIF_SIZE/4 - SHIP_WIDTH/2;
    local_persist U32 y = GIF_SIZE - SHIP_START_Y;
    float wave_offset = WAVE_AMP * sin((float)frames * WAVE_SPEED);
    drawbox(x, y+wave_offset, SHIP_WIDTH, SHIP_HEIGHT, 0x231509);
    drawbox(x, y+SHIP_HEIGHT+wave_offset, SHIP_WIDTH, SHIP_HEIGHT/2, 0x121230);
    drawstringn("SS AOC", 200, y+SHIP_HEIGHT/2+20+wave_offset, 2, 0x9bbcdc);
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

void push_crate(Node **stacks, U32 index, Node *crate) {
    if (!crate) return;
    Node *stack = stacks[index];
    crate->next = stack;
    stacks[index] = crate;
}


void part_one(Node **stacks, size_t stack_count, Instruction *instructions, size_t instruction_count) {
    size_t i, j, src, dest;
    Node *crate;
    local_persist int delay = 1;

    for (i=0; i<instruction_count; ++i) {
        src = instructions[i].src-1;
        dest = instructions[i].dest-1;

        if (frames >  100) delay = 2;
        if (frames >  200) delay = 4;
        if (frames >  600) delay = 8;
        if (frames >  800) delay = 16;
        if (frames >  1000) delay = 32;


        for (j=0; j < instructions[i].count; ++j) {
            crate = pop_crate(stacks, src);
            push_crate(stacks, dest, crate);

            if ((frames % delay) == 0)
                clear();
            draw_bg();
            draw_stacks(stacks, stack_count);
            draw_ship();
            if ((frames % delay) == 0) {
                /*drawstringf(25, 25, 5, 0x000000, "%u", frames);*/
                nextframe(0);
            }

            ++frames;
        }
    }
    /*drawstringf(25, 25, 5, 0x000000, "%u", frames);*/
    nextframe(0);

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
    local_persist int delay = 1;

    for (i=0; i<instruction_count; ++i) {
        src = instructions[i].src-1;
        dest = instructions[i].dest-1;
        head = stacks[src];

        if (frames > 100) delay = 2;
        if (frames > 200) delay = 4;
        if (frames > 300) delay = 8;
        if (frames > 400) delay = 16;
        if (frames > 600) delay = 32;


        node = stacks[src];
        for (j=1; j < instructions[i].count; ++j) {
            assert(node);
            node = node->next;
        }

        stacks[src] = node->next;
        node->next = stacks[dest];
        stacks[dest] = head;

        if ((frames % delay) == 0)
            clear();
        draw_bg();
        draw_stacks(stacks, stack_count);
        draw_ship();
        if ((frames % delay) == 0) {
            /*drawstringf(25, 25, 5, 0x000000, "%u", frames);*/
            nextframe(0);
        }

        ++frames;
    }
    /*drawstringf(25, 25, 5, 0x000000, "%u", frames);*/
    nextframe(0);

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


    setupgif(0, 2, "stacks.gif");

    part_one(stacks, stack_count, instructions, instruction_count);
    /*part_two(stacks, stack_count, instructions, instruction_count);*/

    endgif();

    return 0;
}

