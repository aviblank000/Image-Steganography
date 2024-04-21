#ifndef DECODE_H
#define DECODE_H

#include "types.h" // Contains user defined types

/*
 * Structure to store information required for
 * decoding stegno Image to get secret data.
 * Info about output and intermediate data is
 * also stored
 */

#define MAX_FILE_SUFFIX 4

typedef struct _DecodeInfo
{
    /* Stego Image info */
    char *src_image_fname;
    FILE *fptr_src_image;

    /* Secret File Info */
    char *secret_fname;
    FILE *fptr_secret;
    char extn_secret_file[MAX_FILE_SUFFIX];
    long size_secret_file;
    long size_extn;

} DecodeInfo;

/* Decoding function prototype */

/* Read and validate Encode args from argv */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);

/* Get File pointers for img file */
Status open_img_file(DecodeInfo *decInfo);

/* Perform the decoding */
Status do_decoding(DecodeInfo *decInfo);

/* Check the magic string */
Status decode_magic_string(FILE *fptr_img);

/* Decoding a byte from the lsb */
char decode_byte_from_lsb(char *buffer);

/* Decoding the size of the file extension */
Status decode_secret_file_ext_size(DecodeInfo *decInfo);

/* Decoding size from lsb*/
long decode_size_from_lsb(char *buffer);

/* Decoding file ext */
Status decode_secret_file_ext(DecodeInfo *decInfo);

/* Decoding file data size*/
Status decode_file_data_size(DecodeInfo *decInfo);

/* Decoding file data */
Status decode_file_data(DecodeInfo *decInfo);

#endif
