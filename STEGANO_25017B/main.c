#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "decode.h"
#include "types.h"

OperationType check_operation_type(char *);//protoype of check_opertion_type function

int main(int argc, char *argv[])
{
    if (argc >= 4 && check_operation_type(argv[1]) == e_encode)//argument count should be equal or more than 4 and on index 1 -e or -d should be their
    {
        EncodeInfo enc_info;//structure variable declaration

        if (read_and_validate_encode_args(argv, &enc_info) == e_success)//checking the passed argument and checking file extension
        {
            if (do_encoding(&enc_info) == e_success)//after checking argument it will call the encoding function
            {
                printf("Encoding Successful!\n");//after completing all the operation of encode displaying prompt msg
                fclose(enc_info.fptr_secret);//closing secret file
                fclose(enc_info.fptr_src_image);//closing source file
                fclose(enc_info.fptr_stego_image);//closing dest file
            }
            else
            {
                printf("Encoding Failed!\n");
                return e_failure;
            }
        }
        else
        {
            printf("Invalid arguments for encoding!\n");//if arguments are incorrect it will terminate
            return e_failure;//terminate the program
        }
    }
    else if (argc >= 3 && check_operation_type(argv[1]) == e_decode)//for decoding argument count should be 3 or more than 3 and at 1st index string "-d" should be their
    {
        DecodeInfo dec_info;//structure variable declaration for decoding

        if (read_and_validate_decode_args(argv, &dec_info) == e_success)//checking the passed argument and checking file extension
        {
            if (do_decoding(&dec_info) == e_success)//after checking argument it will call the decoding function
            {
                printf("Decoding Successful!\n");//after completing all the operation of decode displaying prompt msg
            }
            else
            {
                printf("Decoding Failed!\n");
                return e_failure;
            }
        }
        else
        {
            printf("Invalid arguments for decoding!\n");//if arguments are incorrect it will terminate
            return e_failure;
        }
    }
    else
    {
        
        return e_failure;//if both endoing and decoding is failed it will terminate the program
    }

    return e_success;
}

// Function to identify operation type
OperationType check_operation_type(char *symbol)
{
    if (strcmp(symbol, "-e") == 0)//for encoding 1st row consist of "-e" string
    {
        return e_encode;
    }
    else if (strcmp(symbol, "-d") == 0)//for decoding 1st row consist of "-d" string
    {
        return e_decode;
    }
    else
    {
        fprintf(stderr, "ERROR: Unsupported operation '%s'\n", symbol);//if that 1st row not consist of "-d" or "-e" it will terminate and show error message
        return e_unsupported;
    }
}
