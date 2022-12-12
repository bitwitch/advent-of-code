#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "../base/base_inc.h"
#include "../base/base_inc.c"

#define GIF_WIDTH  1024
#define GIF_HEIGHT 1024

typedef enum { 
    MARKER_MODE_PACKET = 0,
    MARKER_MODE_MESSAGE,
    MARKER_MODE_COUNT
} Marker_Mode;

U32 rgb_to_bgr(U32 color) {
    U32 r, g, b;
    b = (color >>  0) & 0xFF;
    g = (color >>  8) & 0xFF;
    r = (color >> 16) & 0xFF;
   return r | (g << 8) | (b << 16);
}

/* NOTE(shaw) this is not trying to be smart at all */
bool all_characters_unique(uint8_t *str, size_t size) { 
    int i;
    for (i=0; i<(int)size-1; ++i)
        if (memchr(str+i+1, str[i], size-i-1))
            return false;
    return true;
}

void draw_device(Marker_Mode mode) {
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
    drawbox(25, GIF_HEIGHT-25-height, GIF_WIDTH-50, height, device_color);
    // antenna
    drawbox(25, GIF_HEIGHT-25-height-antenna_height, 50, antenna_height, device_color);
    // vents
    for (i=0; i<vent_rows; ++i)
        drawbox(75, 715+i*(vent_height+vent_height), 200, vent_height, vent_color);
    // dial
    drawcircle(790, 817, 128, rgb_to_bgr(0x8a3a0f));
    drawcircle(790, 820, 120, rgb_to_bgr(0xface4b));
    if (mode == MARKER_MODE_PACKET) {
        drawcircle(730, 815, 40, rgb_to_bgr(0xc9a436));
        drawcircle(730, 815, 20, rgb_to_bgr(0xface4b));
    } else {
        drawcircle(830, 776, 40, rgb_to_bgr(0xc9a436));
        drawcircle(830, 776, 20, rgb_to_bgr(0xface4b));
    }
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
    drawbox(25+40, GIF_HEIGHT-25-height + 75, GIF_WIDTH-50 - 80, 220,  /* 0x000000); */
            rgb_to_bgr(0x00f058));

}

void draw_detector_led(bool on) {
    drawcircle(480, 780, 22, 0x333333);
    drawcircle(480, 780, 18, on ? 0x00FF00 : 0x000013);
}


void draw_stream_with_window(uint8_t *stream, size_t input_size, size_t window_start, size_t window_size, U32 character_count, bool detected) {
    U32 display_text_color = detected ? 0x2c2c2c : 0x232323; 

    Marker_Mode marker_mode = MARKER_MODE_MESSAGE;
    if (window_size == 4) {
        marker_mode = MARKER_MODE_PACKET;
    }

    draw_device(marker_mode);

    // find how many characters or rects we can fit on screen
    U32 total_chars = 48;
    U32 window_x = 70;

    // draw that many
    if (window_start > input_size) return;

    if (window_start + window_size > input_size) {
        total_chars = input_size - window_start;
    }

    // draw byte stream floating through the air
    drawstringf(80, 125, 3, 0xFFFFFF, "%.*s", total_chars, stream+window_start+window_size);

    // draw current slice of byte stream being checked on device
    if (marker_mode == MARKER_MODE_PACKET) window_x = 700;
    drawstringf(window_x, GIF_HEIGHT-550, 8, 
            display_text_color, 
            "%.*s", window_size, stream+window_start);

    // draw character count
    drawstringf(100, 610, 2, display_text_color, "COUNT: %d", character_count);
    // draw marker mode
    drawstringf(360, 610, 2, display_text_color, "MODE: %s", marker_mode == MARKER_MODE_PACKET ? "PACKET" : "MESSAGE");
    // draw marker length
    drawstringf(660, 610, 2, display_text_color, "MARKER LENGTH: %d", window_size);
    // draw detector LED
    draw_detector_led(detected);
}


U32 characters_processed_before_marker_detected(uint8_t *input, size_t input_size, size_t marker_length) {
    size_t i;
    U32 count = 0;
    local_persist U32 frame_count = 0;
    bool detected = false;
    U32 delay = 32;

    for (i=0; i < input_size - marker_length; ++i) {
        detected = false;
        count = i + marker_length;

        if ((frame_count % delay) == 0) {
            clear();
            // draw window we are currently checking
            draw_stream_with_window(input, input_size, i, marker_length, count, detected);
            nextframe(0);
        }

        if (all_characters_unique(input+i, marker_length)) {
            // draw detector LED
            detected = true;
            for (int j=0; j<20; ++j) {
                clear();
                draw_stream_with_window(input, input_size, i, marker_length, count, detected);
                nextframe(0);
            }
            break;
        }
        ++frame_count;
    }

    clear();
    // draw window we are currently checking
    draw_stream_with_window(input, input_size, i, marker_length, count, detected);
    nextframe(0);

    return count;
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

    setupgif(0, 1, "tuning.gif");

    U32 part_one = 0, part_two = 0;
    part_one = characters_processed_before_marker_detected(file_data, file_size, 4);
    part_two = characters_processed_before_marker_detected(file_data, file_size, 14);

    endgif();

    printf("part one: %d\npart two: %d\n", part_one, part_two);

    return 0;
}
