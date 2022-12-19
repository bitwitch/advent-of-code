#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "../../base/base_inc.h"
#include "../../base/base_inc.c"

#define GIF_WIDTH  (1024)
#define GIF_HEIGHT (1024)
#define CRT_ROWS 6
#define CRT_COLS 40

#define PIXEL_WIDTH    21
#define PIXEL_HEIGHT   40
#define PIXEL_MARGIN_X 2
#define PIXEL_MARGIN_Y 2
#define SCANLINE_COLOR 0xFFFFFF

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

U32 rgb_to_bgr(U32 color) {
    U32 r, g, b;
    b = (color >>  0) & 0xFF;
    g = (color >>  8) & 0xFF;
    r = (color >> 16) & 0xFF;
   return r | (g << 8) | (b << 16);
}

void draw_device(void) {
    int height = 640;
    int antenna_height = 300;

    U32 device_color = rgb_to_bgr(0xc21111);

    int buttons_x = 370;
    int buttons_y = 690;
    int pad_x = 35;
    int pad_y = 20;
    int button_height = 30;
    int button_width = 90;
    int button_rows = 1;
    int button_cols = 2;
    U32 button_color = 0x232323;

    int vent_rows = 6;
    int vent_height = 20;
    U32 vent_color = rgb_to_bgr(0x300008);

    int i, j;
    // device body
    drawbox(25, GIF_HEIGHT-50-height, GIF_WIDTH-50, height, device_color);
    // antenna
    drawbox(25, GIF_HEIGHT-50-height-antenna_height, 50, antenna_height, device_color);
    // vents
    for (i=0; i<vent_rows; ++i)
        drawbox(75, 715+i*(vent_height+vent_height), 200, vent_height, vent_color);
    // dial
    drawcircle(790, 817, 128, rgb_to_bgr(0x8a3a0f));
    drawcircle(790, 820, 120, rgb_to_bgr(0xface4b));
    drawcircle(730, 815, 40, rgb_to_bgr(0xc9a436));
    drawcircle(730, 815, 20, rgb_to_bgr(0xface4b));
    // buttons
    for (j=0; j<button_rows; ++j)
    for (i=0; i<button_cols; ++i)
        drawbox(buttons_x + i*(button_width+pad_x),
                buttons_y + j*(button_height+pad_y),
                button_width, button_height,
                button_color);
    // two small dials
    drawcircle(390, 870, 50, 0x232323);
    drawline(390, 865, 390, 865-35, 3, 0x666666);
    drawcircle(560, 870, 50, 0x232323);
    drawline(558, 868, 538, 848, 3, 0x666666);
    // display
    drawbox(25+30, 390, GIF_WIDTH-50 - 56, 250, 0x000000);
            /*rgb_to_bgr(0x00f058));*/
}

void render_gif(char screen[CRT_ROWS][CRT_COLS]) {
    int i, j, x, y, scan_x, scan_y, screen_x, screen_y;

    draw_device();

    screen_x = 25+30;
    screen_y = 390;

    for (j=0; j<CRT_ROWS; ++j)
    for (i=0; i<CRT_COLS; ++i)
    {
        x = screen_x + i * (PIXEL_WIDTH  + PIXEL_MARGIN_X);
        y = screen_y + j * (PIXEL_HEIGHT + PIXEL_MARGIN_Y);

        // draw scanline 
        scan_x = screen_x + (i+1) * (PIXEL_WIDTH  + PIXEL_MARGIN_X);
        scan_y = y;
        if (i < CRT_COLS - 1)
            drawbox(scan_x, scan_y, PIXEL_WIDTH, PIXEL_HEIGHT, 0xFFFFFF);

        // draw pixel
        if (screen[j][i] == '#') {
            drawbox(x, y, PIXEL_WIDTH/3, PIXEL_HEIGHT, 0x0000FF); // red
            x += PIXEL_WIDTH/3;
            drawbox(x, y, PIXEL_WIDTH/3, PIXEL_HEIGHT, 0x00FF00); // green
            x += PIXEL_WIDTH/3;
            drawbox(x, y, PIXEL_WIDTH/3, PIXEL_HEIGHT, 0xFF0000); // blue
        } else {
            drawbox(x, y, PIXEL_WIDTH, PIXEL_HEIGHT, 0x000000);
        }

        nextframe();
    }
}

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

    render_gif(screen);

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

    setupgif(0, 1, "crt.gif");

    part_one(instructions);
    part_two(instructions);

    endgif();

    return 0;
}
