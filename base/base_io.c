#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include "base_io.h"

int arb_read_entire_file(FILE *fp, uint8_t **dataptr, size_t *sizeptr) {

    /*
     * See answer by Nominal Animal (note this is not the accepted answer)
     * https://stackoverflow.com/questions/14002954/c-programming-how-to-read-the-whole-file-contents-into-a-buffer#answer-44894946
     */

    uint8_t *data = NULL, *temp;
    uint64_t bytes_allocated = 0;
    uint64_t read_so_far = 0;
    uint64_t n; // bytes read in a single fread call

    /* None of the parameters can be NULL. */
    if (fp == NULL || dataptr == NULL || sizeptr == NULL)
        return ARB_READ_ENTIRE_FILE_INVALID;

    /* A read error already occurred? */
    if (ferror(fp))
        return ARB_READ_ENTIRE_FILE_ERROR;

    while (1) {
        /* first check if buffer is large enough to read another chunk */
        uint64_t new_size = read_so_far + ARB_READFILE_CHUNK + 1;

        if (bytes_allocated < new_size) {
            /* need to grow the buffer */
            bytes_allocated = new_size;

            /* overflow check */
            if (new_size <= read_so_far) {
                free(data);
                return ARB_READ_ENTIRE_FILE_TOOMUCH;
            }

            temp = realloc(data, new_size);
            if (!temp) {
                free(data);
                return ARB_READ_ENTIRE_FILE_NOMEM;
            }
            data = temp;
        }

        /* read in a chunk */
        n = fread(data+read_so_far, sizeof(uint8_t), ARB_READFILE_CHUNK, fp);
        if (n == 0)
            break;

        read_so_far += n;
    }

    if (ferror(fp)) {
        free(data);
        return ARB_READ_ENTIRE_FILE_ERROR;
    }

    /* resize the buffer to the exact length of the file (plus 1 for null termination) */
    temp = realloc(data, read_so_far + 1);
    if (!temp) {
        free(data);
        return ARB_READ_ENTIRE_FILE_NOMEM;
    }
    data = temp;
    data[read_so_far] = '\0';

    *dataptr = data;
    *sizeptr = read_so_far;
    return ARB_READ_ENTIRE_FILE_OK;
}


char *arb_chop_by_delimiter(char **str, char *delimiter) {
    char *chopped = *str;

    char *found = strstr(*str, delimiter);
    if (found == NULL) {
        str += strlen(*str);
        return chopped;
    }

    *found = '\0';
    *str = found + strlen(delimiter);

    return chopped;
}

