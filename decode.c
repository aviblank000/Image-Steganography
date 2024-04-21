#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "decode.h"
#include "encode.h"
#include "types.h"
#include "common.h"

/* Function Definitions */
 
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    if (strstr(argv[2], ".bmp"))
	decInfo -> src_image_fname = argv[2];
    else
	return d_failure;
    
    decInfo -> secret_fname = malloc(12);
    
    if (argv[3])
	strcpy(decInfo -> secret_fname, argv[3]);
    else
	strcpy(decInfo -> secret_fname, "output");
    
    return d_success;
}

Status open_img_file(DecodeInfo *decInfo)
{
    // Stego Image file
    decInfo -> fptr_src_image = fopen(decInfo->src_image_fname, "r");
    // Do Error handling
    if (decInfo -> fptr_src_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->src_image_fname);

        return d_failure;
    }
    // No failure return e_success
    return d_success;
}

Status do_decoding(DecodeInfo *decInfo)
{
    if (open_img_file(decInfo) == d_success)
	printf("Open file is successfull\n");
    else
    {
	printf("open file was a failure\n");
	return d_failure;
    }

    if (decode_magic_string(decInfo -> fptr_src_image) == d_success) 
	printf("Decode magic string is successfull\n");
    else
    {
	printf("Decode magic string was a failure\n");
	return d_failure;
    }
    
    if (decode_secret_file_ext_size(decInfo) == d_success) 
	printf("Decoding secret file ext size is successfull\n");
    else
    {
	printf("Decoding secret file ext size was a failure\n");
	return d_failure;
    }
    
    if (decode_secret_file_ext(decInfo) == d_success) 
	printf("Decoding secret file ext is successfull\n");
    else
    {
	printf("Decoding secret file ext was a failure\n");
	return d_failure;
    }
    
    if (decode_file_data_size(decInfo) == d_success) 
	printf("Decoding secret file data size is successfull\n");
    else
    {
	printf("Decoding secret file data size was a failure\n");
	return d_failure;
    }
    
    if (decode_file_data(decInfo) == d_success) 
	printf("Decoding secret file data is successfull\n");
    else
    {
	printf("Decoding secret file data was a failure\n");
	return d_failure;
    }
    
    return d_success;
}

Status decode_magic_string(FILE *fptr_img)
{
    char buffer[8]; 
    // array to hold the magic string.
    char *ch = malloc(strlen(MAGIC_STRING));

    // moving image pointer to 54 byte.
    fseek(fptr_img, 54, SEEK_SET);
    
    //getting the magic string.
    for (int i = 0; i < strlen(MAGIC_STRING); i++)
    {
	fread(buffer, 8, 1, fptr_img);
	ch[i] = decode_byte_from_lsb(buffer);
    }

    // checking magic string.
    if (strcmp(ch, MAGIC_STRING) == 0)
	return d_success;
    else
	return d_failure;
}

char decode_byte_from_lsb(char *buffer)
{
    // store the character.
    char ch = 0;
    // Retrieving the lsb bit, moving and storing in ch.
    for (int i = 0 ; i < 8; i++)
	ch = (buffer[i] & 1) | (ch << 1);
    
    return ch;
}

Status decode_secret_file_ext_size(DecodeInfo *decInfo)
{
    char buffer[32];
    // reading 32 bytes from source image.
    fread(buffer, 32, 1, decInfo -> fptr_src_image);
    // getting the size and storing in the structure.
    decInfo -> size_extn = decode_size_from_lsb(buffer);

    return d_success;
}

long decode_size_from_lsb(char *buffer)
{
    long size = 0;
    // Retrieving the lsb bit, moving and storing in size.
    for (int i = 0; i < 32; i++)
	size = (buffer[i] & 1) | (size << 1);

    return size; 
}

Status decode_secret_file_ext(DecodeInfo *decInfo)
{
    char buffer[8];

    for (int i = 0; i < decInfo -> size_extn; i++)
    {
	fread(buffer, 8, 1, decInfo -> fptr_src_image);
	// getting the character of the extension.
	decInfo -> extn_secret_file[i] = decode_byte_from_lsb(buffer);
    }
    // Merge the extension with file name and open the file.
    decInfo -> fptr_secret = fopen(strcat(decInfo -> secret_fname, (char *)decInfo -> extn_secret_file), "w");

    // Do Error handling
    if (decInfo -> fptr_secret  == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->secret_fname);
        return d_failure;
    }
    
    return d_success;
}

Status decode_file_data_size(DecodeInfo *decInfo)
{
    char buffer[32];
    
    // reading 32 bytes from source image.
    fread(buffer, 32, 1, decInfo -> fptr_src_image);
    // getting the size and storing in the structure.
    decInfo -> size_secret_file = decode_size_from_lsb(buffer);

    return d_success;
}

Status decode_file_data(DecodeInfo *decInfo)
{
    char buffer[8], ch;

    for (int i = 0; i < decInfo -> size_secret_file - 1; i++)
    {
	// reading 8 bytes from source image.
	fread(buffer, 8, 1, decInfo -> fptr_src_image);
	// decoding one character.
	ch = decode_byte_from_lsb(buffer);
	// storing character in output file.
	fwrite(&ch, 1, 1, decInfo -> fptr_secret);
    }
    ch = '\n';
    fwrite(&ch, 1, 1, decInfo -> fptr_secret);

    return d_success;
}
