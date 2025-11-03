#ifndef DECODE_H       // Header guard to prevent multiple inclusion of this file
#define DECODE_H

#include <stdio.h>     // Standard I/O header for file handling
#include "types.h"     // Custom header file for type definitions (e.g., Status enum)

/* Structure to store all decoding-related information */
typedef struct _DecodeInfo
{
    /* Secret File Info */
    char *secret_fname;             // Name of the output decoded secret file
    FILE *fptr_secret;              // File pointer to write the decoded secret data
    char extn_secret_file[10];      // Extension of the secret file (e.g., .txt, .c)
    long size_secret_file;          // Size of the secret file in bytes
    int extn_size;                  // Size of the file extension (number of characters)

    /* Stego Image Info */
    char *stego_image_fname;        // Name of the input stego image file (.bmp)
    FILE *fptr_stego_image;         // File pointer to read stego image data
} DecodeInfo;

/* Function Prototypes */

/* Reads and validates command-line arguments for decoding */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);

/* Main decoding function that coordinates all decoding steps */
Status do_decoding(DecodeInfo *decInfo);

/* Opens the stego image file for reading */
Status open_files_d(DecodeInfo *decInfo);

/* Skips the 54-byte BMP header in the image file */
Status skip_bmp_header(FILE *fptr);

/* Decodes and verifies the magic string from the stego image */
Status decode_magic_string(const char *magic_string, DecodeInfo *decInfo);

/* Decodes the size of the secret file extension */
Status decode_secret_file_extn_size(int *size, DecodeInfo *decInfo);

/* Decodes the secret file extension (e.g., ".txt") */
Status decode_secret_file_extn(DecodeInfo *decInfo);

/* Decodes the total size of the secret file */
Status decode_secret_file_size(long *size, DecodeInfo *decInfo);

/* Decodes the actual secret data and writes it to an output file */
Status decode_secret_file_data(DecodeInfo *decInfo);

/* Decodes one byte from the least significant bits (LSBs) of 8 image bytes */
Status decode_byte_from_lsb(char *data, char *image_buffer);

/* Decodes a 32-bit integer (like file size) from the LSBs of image bytes */
Status decode_size_from_lsb(int *size, char *image_buffer);

#endif   // End of header guard
