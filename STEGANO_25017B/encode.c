#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "types.h"
#include "common.h"

/* ---------- Function Definitions for Encoding Process ---------- */

int extn_size; // Global variable to hold secret file extension size

/* 
 * Function: get_image_size_for_bmp
 * --------------------------------
 * Reads the width and height from a BMP image header to calculate image capacity.
 * Each pixel in BMP is represented using 3 bytes (R, G, B).
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;

    // Move the file pointer to the 18th byte in the BMP header (width position)
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (4 bytes)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (4 bytes)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return total image size (width * height * 3 bytes per pixel)
    return width * height * 3;
}

/*
 * Function: get_file_size
 * -----------------------
 * Returns the size of a given file in bytes.
 */
uint get_file_size(FILE *fptr)
{
    fseek(fptr, 0, SEEK_END); // Move to end of file
    return ftell(fptr);       // Return current position (file size)
}

/*
 * Function: read_and_validate_encode_args
 * ---------------------------------------
 * Validates the command-line arguments for encoding.
 * Checks if source image, secret file, and output file names are valid.
 */
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    // Validate source image file
    if (argv[2] == NULL)
    {
        fprintf(stderr, "ERROR: Source image file not provided.\n");
        return e_failure;
    }

    // Check if name is missing before dot
    if (argv[2][0] == '.')
    {
        fprintf(stderr, "ERROR: File name is not present before '.' in source image.\n");
        return e_failure;
    }

    // Check file extension
    if (strstr(argv[2], ".bmp") != NULL)
        encInfo->src_image_fname = argv[2];
    else
    {
        fprintf(stderr, "ERROR: Invalid source file. Must end with '.bmp'\n");
        return e_failure;
    }

    // Validate secret file
    if (argv[3] == NULL)
    {
        fprintf(stderr, "ERROR: Secret file not provided.\n");
        return e_failure;
    }

    if (argv[3][0] == '.')
    {
        fprintf(stderr, "ERROR: File name is not present before '.' in secret file.\n");
        return e_failure;
    }

    // Acceptable secret file extensions
    if ((strstr(argv[3], ".txt") != NULL) || (strstr(argv[3], ".c") != NULL) ||
        (strstr(argv[3], ".sh") != NULL) || (strstr(argv[3], ".h") != NULL))
    {
        encInfo->secret_fname = argv[3];
    }
    else
    {
        fprintf(stderr, "ERROR: Invalid secret file. Must end with '.txt', '.c', '.h', or '.sh'\n");
        return e_failure;
    }

    // Optional destination file name
    if (argv[4] == NULL)
    {
        encInfo->stego_image_fname = "stego.bmp";  // Default output file name
    }
    else
    {
        if (argv[4][0] == '.')
        {
            fprintf(stderr, "ERROR: File name is not present before '.' in destination file.\n");
            return e_failure;
        }

        if (strstr(argv[4], ".bmp") != NULL)
        {
            encInfo->stego_image_fname = argv[4];
        }
        else
        {
            fprintf(stderr, "ERROR: Invalid destination file. Must end with '.bmp'\n");
            return e_failure;
        }
    }

    return e_success;
}

/*
 * Function: open_files
 * --------------------
 * Opens source image, secret, and destination (stego) files for processing.
 */
Status open_files(EncodeInfo *encInfo)
{
    // Open source image
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    if (encInfo->fptr_src_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);
        return e_failure;
    }

    // Open secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    if (encInfo->fptr_secret == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);
        return e_failure;
    }

    // Open stego image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    if (encInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);
        return e_failure;
    }

    return e_success;
}

/*
 * Function: check_capacity
 * ------------------------
 * Checks if the source image has enough capacity to hide the secret file.
 */
Status check_capacity(EncodeInfo *encInfo)
{
    // Get total bytes available in image
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);

    // Get secret file size
    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);

    // Get file extension of secret file
    char *extn;
    if (strstr(encInfo->secret_fname, ".txt") != NULL)
        extn = strstr(encInfo->secret_fname, ".txt");
    else if (strstr(encInfo->secret_fname, ".c") != NULL)
        extn = strstr(encInfo->secret_fname, ".c");
    else if (strstr(encInfo->secret_fname, ".sh") != NULL)
        extn = strstr(encInfo->secret_fname, ".sh");
    else if (strstr(encInfo->secret_fname, ".h") != NULL)
        extn = strstr(encInfo->secret_fname, ".h");

    // Store extension and calculate its size
    strcpy(encInfo->extn_secret_file, extn);
    extn_size = strlen(extn);

    // Calculate total required bytes for embedding
    int total_bytes = 54 + (strlen(MAGIC_STRING) * 8) + 32 + (extn_size * 8) + 32 + (encInfo->size_secret_file * 8);

    // Compare capacity and required bytes
    if (encInfo->image_capacity > total_bytes)
        return e_success;
    else
        return e_failure;
}

/*
 * Function: copy_bmp_header
 * -------------------------
 * Copies the first 54 bytes (BMP header) from source image to destination file.
 */
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    rewind(fptr_src_image);
    unsigned char header[54];  // BMP header is 54 bytes
    fread(header, sizeof(char), 54, fptr_src_image);
    fwrite(header, sizeof(char), 54, fptr_dest_image);

    if (ftell(fptr_dest_image) == ftell(fptr_src_image))
        return e_success;
    else
        return e_failure;
}

/*
 * Function: encode_magic_string
 * -----------------------------
 * Embeds a predefined "magic string" (used to identify stego images).
 */
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    char buffer[8];
    for (size_t i = 0; i < strlen(magic_string); i++)
    {
        fread(buffer, 8, 1, encInfo->fptr_src_image);
        encode_byte_to_lsb(magic_string[i], buffer);
        fwrite(buffer, 8, 1, encInfo->fptr_stego_image);
    }
    return e_success;
}

/*
 * Function: encode_secret_file_extn_size
 * -------------------------------------
 * Stores the size of the secret file’s extension in the image (32 bits).
 */
Status encode_secret_file_extn_size(int size, EncodeInfo *encInfo)
{
    char buffer[32];
    fread(buffer, 32, 1, encInfo->fptr_src_image);
    encode_size_to_lsb(size, buffer);
    fwrite(buffer, 32, 1, encInfo->fptr_stego_image);
    return e_success;
}

/*
 * Function: encode_secret_file_extn
 * ---------------------------------
 * Encodes the secret file’s extension (like .txt or .c) into the image.
 */
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    char buffer[8];
    for (size_t i = 0; i < strlen(file_extn); i++)
    {
        fread(buffer, 8, 1, encInfo->fptr_src_image);
        encode_byte_to_lsb(file_extn[i], buffer);
        fwrite(buffer, 8, 1, encInfo->fptr_stego_image);
    }
    return e_success;
}

/*
 * Function: encode_secret_file_size
 * ---------------------------------
 * Embeds the size of the secret file (in bytes) into the image.
 */
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    char buffer[32];
    fread(buffer, 32, 1, encInfo->fptr_src_image);
    encode_size_to_lsb(file_size, buffer);
    fwrite(buffer, 32, 1, encInfo->fptr_stego_image);
    return e_success;
}

/*
 * Function: encode_secret_file_data
 * ---------------------------------
 * Encodes the actual content of the secret file into the image data.
 */
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    rewind(encInfo->fptr_secret); // Reset file pointer
    fread(encInfo->secret_data, encInfo->size_secret_file, 1, encInfo->fptr_secret);

    char buffer[8];
    for (size_t i = 0; i < encInfo->size_secret_file; i++)
    {
        fread(buffer, 8, 1, encInfo->fptr_src_image);
        encode_byte_to_lsb(encInfo->secret_data[i], buffer);
        fwrite(buffer, 8, 1, encInfo->fptr_stego_image);
    }
    return e_success;
}

/*
 * Function: copy_remaining_img_data
 * ---------------------------------
 * Copies the remaining bytes of the source image (after encoding) to the output image.
 */
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char ch;
    while (fread(&ch, sizeof(char), 1, fptr_src) == 1)
    {
        fwrite(&ch, sizeof(char), 1, fptr_dest);
    }
    return e_success;
}

/*
 * Function: encode_byte_to_lsb
 * ----------------------------
 * Encodes a single byte (character) into the least significant bits of 8 bytes of image data.
 */
Status encode_byte_to_lsb(char data, char *image_buffer)
{
    for (size_t i = 0; i < 8; i++)
    {
        image_buffer[i] = image_buffer[i] & (~1) | (data >> i) & 1;
    }
    return e_success;
}

/*
 * Function: encode_size_to_lsb
 * ----------------------------
 * Encodes a 32-bit integer value into 32 bytes of image data.
 */
Status encode_size_to_lsb(int size, char *imageBuffer)
{
    for (size_t i = 0; i < 32; i++)
    {
        imageBuffer[i] = imageBuffer[i] & (~1) | (size >> i) & 1;
    }
    return e_success;
}

/*
 * Function: do_encoding
 * ---------------------
 * Master function that controls the entire encoding process step by step.
 */
Status do_encoding(EncodeInfo *encInfo)
{
    if (open_files(encInfo) == e_success)
    {
        if (check_capacity(encInfo) == e_success)
        {
            if (copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
            {
                if (encode_magic_string(MAGIC_STRING, encInfo) == e_success)
                {
                    if (encode_secret_file_extn_size(extn_size, encInfo) == e_success)
                    {
                        if (encode_secret_file_extn(encInfo->extn_secret_file, encInfo) == e_success)
                        {
                            if (encode_secret_file_size(get_file_size(encInfo->fptr_secret), encInfo) == e_success)
                            {
                                if (encode_secret_file_data(encInfo) == e_success)
                                {
                                    if (copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
                                    {
                                        return e_success;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
