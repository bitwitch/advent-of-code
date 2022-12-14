#pragma once 

// this is the size of a chunk of data in each read. one is added to this in
// the actual call to fread to leave space for a null character
#ifndef  ARB_READFILE_CHUNK
#define ARB_READFILE_CHUNK 2097152 // 2MiB
#endif

#define  ARB_READ_ENTIRE_FILE_OK          0  /* Success */
#define  ARB_READ_ENTIRE_FILE_INVALID    -1  /* Invalid parameters */
#define  ARB_READ_ENTIRE_FILE_ERROR      -2  /* Stream error */
#define  ARB_READ_ENTIRE_FILE_TOOMUCH    -3  /* Too much input */
#define  ARB_READ_ENTIRE_FILE_NOMEM      -4  /* Out of memory */

typedef enum {
    ARB_MODE_READ_TEXT,
    ARB_MODE_READ_BINARY
} arb_File_Read_Mode;

int arb_read_entire_file(FILE *fp, uint8_t **dataptr, size_t *sizeptr);

char *arb_chop_by_delimiter(char **str, char *delimiter);
