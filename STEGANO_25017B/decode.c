#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "decode.h"
#include "types.h"
#include "common.h"

/* 
 * Function: read_and_validate_decode_args
 * ---------------------------------------
 * Validates the command-line arguments passed for decoding.
 * Arguments:
 *   argv[] : command-line arguments
 *   decInfo: structure holding decoding info
 * Returns:
 *   e_success if validation passes, otherwise e_failure
 */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    // Validate stego image file (must be .bmp)
    if (strstr(argv[2], ".bmp") != NULL)
        decInfo->stego_image_fname = argv[2];
    else
    {
        fprintf(stderr, "ERROR: Invalid stego image. Must end with .bmp\n");
        return e_failure;
    }

    // Optional output filename for the decoded secret
    if (argv[3] != NULL)
        decInfo->secret_fname = argv[3];
    else
        decInfo->secret_fname = "decoded_output"; // default name if not given

    return e_success;
}

/*
 * Function: open_files_d
 * ----------------------
 * Opens the stego (encoded) image file for reading.
 */
Status open_files_d(DecodeInfo *decInfo)
{
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "r");
    if (!decInfo->fptr_stego_image)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open stego image %s\n", decInfo->stego_image_fname);
        return e_failure;
    }
    return e_success;
}

/*
 * Function: skip_bmp_header
 * -------------------------
 * Skips the 54-byte BMP header since actual pixel data starts after it.
 */
Status skip_bmp_header(FILE *fptr)
{
    fseek(fptr, 54, SEEK_SET);
    return e_success;
}

/*
 * Function: decode_byte_from_lsb
 * ------------------------------
 * Decodes one byte (character) from 8 pixels (LSBs of 8 bytes).
 */
Status decode_byte_from_lsb(char *data, char *image_buffer)
{
    *data = 0;
    for (int i = 0; i < 8; i++)
        *data |= (image_buffer[i] & 1) << i;  // extract LSBs and combine into a byte
    return e_success;
}

/*
 * Function: decode_size_from_lsb
 * ------------------------------
 * Decodes a 32-bit integer (size) from 32 pixels (LSBs of 32 bytes).
 */
Status decode_size_from_lsb(int *size, char *image_buffer)
{
    *size = 0;
    for (int i = 0; i < 32; i++)
        *size |= (image_buffer[i] & 1) << i;  // reconstruct integer bit-by-bit
    return e_success;
}

/*
 * Function: decode_magic_string
 * -----------------------------
 * Decodes and verifies the magic string ("##") to confirm encoding validity.
 */
Status decode_magic_string(const char *magic_string, DecodeInfo *decInfo)
{
    char buffer[8];
    char ch;
    char magic_str[3];  // buffer to store decoded magic string

    for (int i = 0; i < 2; i++) // read 2 characters (##)
    {
        fread(buffer, 8, 1, decInfo->fptr_stego_image);
        decode_byte_from_lsb(&ch, buffer);
        magic_str[i] = ch;
    }
    magic_str[2] = '\0';  // null terminate

    // Compare with expected magic string
    if (strcmp(magic_str, MAGIC_STRING) != 0)
    {
        fprintf(stderr, "ERROR: This image is not encoded properly!\n");
        return e_failure;
    }

    return e_success;
}

/*
 * Function: decode_secret_file_extn_size
 * --------------------------------------
 * Decodes the size (number of characters) of the secret file’s extension.
 */
Status decode_secret_file_extn_size(int *size, DecodeInfo *decInfo)
{
    char buffer[32];
    fread(buffer, 32, 1, decInfo->fptr_stego_image); // read 32 bytes
    decode_size_from_lsb(size, buffer);              // extract size
    decInfo->extn_size = *size;
    return e_success;
}

/*
 * Function: decode_secret_file_extn
 * ---------------------------------
 * Decodes the extension (e.g., ".txt", ".c") of the secret file.
 */
Status decode_secret_file_extn(DecodeInfo *decInfo)
{
    char buffer[8];
    char ch;
    for (int i = 0; i < decInfo->extn_size; i++)  // decode character by character
    {
        fread(buffer, 8, 1, decInfo->fptr_stego_image);
        decode_byte_from_lsb(&ch, buffer);
        decInfo->extn_secret_file[i] = ch;
    }
    decInfo->extn_secret_file[decInfo->extn_size] = '\0'; // null terminate
    return e_success;
}

/*
 * Function: decode_secret_file_size
 * ---------------------------------
 * Decodes the total size (in bytes) of the secret file content.
 */
Status decode_secret_file_size(long *size, DecodeInfo *decInfo)
{
    char buffer[32];
    fread(buffer, 32, 1, decInfo->fptr_stego_image);      // read 32 bytes
    decode_size_from_lsb((int *)size, buffer);            // extract file size
    decInfo->size_secret_file = *size;
    return e_success;
}

/*
 * Function: decode_secret_file_data
 * ---------------------------------
 * Decodes the actual data (contents) of the secret file from the stego image.
 */
Status decode_secret_file_data(DecodeInfo *decInfo)
{
    char buffer[8];
    char ch;
    char output_fname[100] = {0};  // buffer to store output filename

    // Copy base output file name
    strcpy(output_fname, decInfo->secret_fname);

    // Append decoded file extension
    strcat(output_fname, decInfo->extn_secret_file);

    // Open output file for writing decoded data
    decInfo->fptr_secret = fopen(output_fname, "w");
    if (!decInfo->fptr_secret)
    {
        perror("fopen");
        return e_failure;
    }

    // Decode each byte of secret file data
    for (int i = 0; i < decInfo->size_secret_file; i++)
    {
        fread(buffer, 8, 1, decInfo->fptr_stego_image);
        decode_byte_from_lsb(&ch, buffer);
        fputc(ch, decInfo->fptr_secret);
    }

    fclose(decInfo->fptr_secret);
    printf("Decoded file created: %s\n", output_fname);
    return e_success;
}

/*
 * Function: do_decoding
 * ---------------------
 * Main decoding process: opens files, skips BMP header, and sequentially decodes
 * magic string, file extension, size, and actual secret data.
 */
Status do_decoding(DecodeInfo *decInfo)
{
    if (open_files_d(decInfo) == e_success)
    {
        skip_bmp_header(decInfo->fptr_stego_image);

        if (decode_magic_string(MAGIC_STRING, decInfo) == e_success)
        {
            decode_secret_file_extn_size(&decInfo->extn_size, decInfo);
            decode_secret_file_extn(decInfo);
            decode_secret_file_size(&decInfo->size_secret_file, decInfo);
            decode_secret_file_data(decInfo);
            return e_success;
        }
    }
    return e_failure;
}
