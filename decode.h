#ifndef DECODE_H
#define DECODE_H
#include "types.h" // Contains user defined types

#define MAX_SECRET_BUF_SIZE 1
#define MAX_IMAGE_BUF_SIZE (MAX_SECRET_BUF_SIZE * 8)
#define MAX_FILE_SUFFIX 4

typedef struct _DecodeInfo
{
    /* Source Image info */
    char *steg_image_fname;
    FILE *fptr_steg_image;
    uint header_size;
    uint image_capacity;

    /* Secret File Info */
    char op_data[1000];
    char secret_fname[100];
    FILE *fptr_secret;
    char extn_secret_file[MAX_FILE_SUFFIX];
    int size_secret_file_extn;
    long size_secret_file;

}DecodeInfo;

uint get_bmp_header_size(FILE *fptr_image);

Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);

Status do_decoding(DecodeInfo *decInfo);

Status decode_magic_string(DecodeInfo *decInfo);

Status decode_secret_file_extn_size(DecodeInfo *decInfo);

Status decode_secret_file_extn(DecodeInfo *decInfo);

Status decode_secret_file_size(DecodeInfo *decInfo);

Status decode_secret_file_data(DecodeInfo *decInfo);

/* Encode function, which does the real encoding */
Status decode_data_from_image(int size,DecodeInfo *decInfo);

/* Encode a byte into LSB of image data array */
char decode_byte_from_lsb(char *image_buffer);



#endif
