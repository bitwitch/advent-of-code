#include <stdio.h>
#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"
#define SV_IMPLEMENTATION
#include "sv.h"

void part_one(void) {

}

void part_two(void) {

}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Must supply filepath to puzzle input as an argument\n");
        exit(1);
    }

    String_View filedata = sv_read_entire_file(argv[1]);

    fread

    String line = chop_by_delimiter(filedata, '\n');

    isspace

    return 0;
}
