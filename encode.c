#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "encode.h"
#include "types.h"
#include "common.h"

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

/* 
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
	perror("fopen");
	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

	return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
	perror("fopen");
	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

	return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
	perror("fopen");
	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

	return e_failure;
    }

    // No failure return e_success
    return e_success;
}

OperationType check_operation_type(char *argv[])
{
    if (strcmp(argv[1], "-e") == 0)
	return e_encode;
    else if (strcmp(argv[1], "-d") == 0)
	return e_decode;
    return e_unsupported;
}

Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    int i;
    char *ext;
    if (strstr(argv[2], ".bmp"))
	encInfo -> src_image_fname = argv[2];
    else
	return e_failure;

    if (ext = strchr(argv[3], '.'))
    {
	encInfo -> secret_fname = argv[3];
	strcpy(encInfo -> extn_secret_file, ext);
    }
    else
	return e_failure;

    if (argv[4])
    {
	if (strstr(argv[4], ".bmp"))
	    encInfo -> stego_image_fname = argv[4];
	else
	{
	    encInfo -> stego_image_fname = malloc(16);
	    encInfo -> stego_image_fname = "stego_image.bmp";
	}

    }

    if (argv[4] == NULL)
    {
	encInfo -> stego_image_fname = malloc(16);
	encInfo -> stego_image_fname = "stego_image.bmp";
    }

    return e_success;   
}

Status do_encoding(EncodeInfo *encInfo)
{
    if (open_files(encInfo) == e_success)
	printf("Open files is successfull\n");
    else
    {
	printf("open files was a failure\n");
	return e_failure;
    }

    if (check_capacity(encInfo) == e_success)
	printf("Check capacity is successfull\n");
    else
    {
	printf("Check capacity was a failure\n");
	return e_failure;
    }  

    if (copy_bmp_header(encInfo -> fptr_src_image, encInfo -> fptr_stego_image) == e_success)
	printf("Copy_bmp_header is successfull\n");
    else
    {
	printf("Copy_bmp_header was a failure\n");
	return e_failure;
    }

    if (encode_magic_string(MAGIC_STRING, encInfo) == e_success)
	printf("Encode_magic_string is successfull\n");
    else
    {
	printf("Encode_magic_string was a failure\n");
	return e_failure;
    }

    if (encode_secret_file_extn_size(strlen(encInfo -> extn_secret_file), encInfo) == e_success)
	printf("Encode secret file extn size is successfull\n");
    else
    {
	printf("Encode secret file extn size was a failure\n");
	return e_failure;
    }

    if (encode_secret_file_extn(encInfo -> extn_secret_file, encInfo) == e_success)
	printf("Encode secret file extn is successfull\n");
    else
    {
	printf("Encode secret file extn was a failure\n");
	return e_failure;
    }

    if (encode_secret_file_size(encInfo -> size_secret_file, encInfo) == e_success)
	printf("Encode secret file size is successfull\n");
    else
    {
	printf("Encode secret file size was a failure\n");
	return e_failure;
    }

    if (encode_secret_file_data(encInfo) == e_success)
	printf("Encode secret file data is successfull\n");
    else
    {
	printf("Encode secret file data was a failure\n");
	return e_failure;
    }
    if (copy_remaining_img_data(encInfo -> fptr_src_image, encInfo -> fptr_stego_image) == e_success)
	printf("copying remaining image data is successfull\n");
    else
    {
	printf("copying remaining image data was a failure\n");
	return e_failure;
    }

    return e_success;
}

Status check_capacity(EncodeInfo *encInfo)
{
    // get image file size.
    uint image_size = get_image_size_for_bmp(encInfo -> fptr_src_image);

    printf("%u\n",image_size);
    // get secret file size.
    encInfo -> size_secret_file = get_file_size(encInfo -> fptr_secret);

    // check capacity - header + magic_string + size_of_extn_secret_file + extn_of_secret_file(char) + size_of_secret_file + content_size_of_the_secret_file.  
    uint cap = 54 + (strlen(MAGIC_STRING) * 8) + (sizeof(int) * 8) + (strlen(encInfo-> extn_secret_file) * 8)  + (sizeof(int) * 8) + ((encInfo -> size_secret_file) * 8); 

    // checking capacity.
    if (cap < image_size)
	return e_success;
    else
	return e_failure;
}

uint get_file_size(FILE *fptr)
{
    // Move file pointer to last and return the size.
    fseek(fptr, 0, SEEK_END);
    return ftell(fptr);
}

Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    // rewinding the source image file pointer to starting of the file.
    rewind(fptr_src_image);
    // buffer to copy the contents.
    char buffer[54];
    // copying the contents.
    fread(buffer, sizeof(char) , 54, fptr_src_image);
    fwrite(buffer, sizeof(char), 54, fptr_dest_image);

    return e_success;
}

Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    encode_data_to_image((char *)magic_string, strlen(magic_string), encInfo -> fptr_src_image, encInfo -> fptr_stego_image);

    return e_success;
}

Status encode_data_to_image(char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image)
{
    // used for copying bytes.
    char buffer[8];

    for (int i = 0; i < size; i++)
    {
	// reading 8 bytes from source image to store 1 byte of secret data.
	fread(buffer, 8, 1, fptr_src_image);
	// function call to encode the lsb in each byte.
	encode_byte_to_lsb(data[i], buffer);
	// writing the encoded data into the stego image.
	fwrite(buffer, 8, 1, fptr_stego_image);
    }
    return e_success;
}

Status encode_byte_to_lsb(char data, char *image_buffer)
{
    // first clearing the lSB bit in the image_buffer and adding the each data bit in the LSB bit using OR operator.
    for (int i = 0; i < 8 ; i++)
	image_buffer[i] = (image_buffer[i] & 0xFE) | (((unsigned) data >> (7 - i)) & 1);
    return e_success;
}

Status encode_secret_file_extn_size(long file_size, EncodeInfo *encInfo)
{
    // used for copying bytes.
    char buffer[32];

    // reading 32 bytes from source image to store file_size.
    fread(buffer, sizeof(buffer), 1, encInfo -> fptr_src_image);
    // function call to encode the lsb in each byte.
    encode_int_to_lsb(file_size, buffer);
    // writing the encoded data into the stego image.
    fwrite(buffer, sizeof(buffer), 1, encInfo -> fptr_stego_image);

    return e_success;
}

Status encode_int_to_lsb(long data, char *image_buffer)
{
    // first clearing the lSB bit in the image_buffer and adding the each data bit in the LSB bit using OR operator.
    for (int i = 0; i < 32 ; i++)
	image_buffer[i] = (image_buffer[i] & 0xFE) | (((unsigned) data >> (31 - i)) & 1);
    return e_success;
}

Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    // used for copying bytes.
    char buffer[8];

    for (int i = 0; i < strlen(file_extn); i++)
    {
	// reading 8 bytes from source image to store 1 char of file_extn data.
	fread(buffer, 8, 1, encInfo -> fptr_src_image);
	// function call to encode the lsb in each byte.
	encode_byte_to_lsb(file_extn[i], buffer);
	// writing the encoded data into the stego image.
	fwrite(buffer, 8, 1, encInfo -> fptr_stego_image);
    }
    return e_success;
}

Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    // used for copying bytes.
    char buffer[32];

    // reading 32 bytes from source image to store file_size.
    fread(buffer, sizeof(buffer), 1, encInfo -> fptr_src_image);
    // function call to encode the lsb in each byte.
    encode_int_to_lsb(file_size, buffer);
    // writing the encoded data into the stego image.
    fwrite(buffer, sizeof(buffer), 1, encInfo -> fptr_stego_image);

    return e_success;
}

Status encode_secret_file_data(EncodeInfo *encInfo)
{
    // used for copying bytes.
    char buffer[8];
    // rewinding the fptr_secret.
    rewind(encInfo -> fptr_secret);

    for (int i = 0; i < (encInfo -> size_secret_file - 1); i++)
    {
	// reading 8 bytes from source image to store 1 char of file_extn data.
	fread(buffer, 8, 1, encInfo -> fptr_src_image);
	// reading secret file information.
	fread(&(encInfo -> secret_data[0]), 1, 1, encInfo -> fptr_secret);
	// function call to encode the lsb in each byte.
	encode_byte_to_lsb(encInfo -> secret_data[0], buffer);
	// writing the encoded data into the stego image.
	fwrite(buffer, 8, 1, encInfo -> fptr_stego_image);
    }
    return e_success;
}

Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char ch;
    // looping through all bytes till the EOF.
    while(fread(&ch, 1, 1, fptr_src))
    {
	if (feof(fptr_src))
	    break;
	fwrite(&ch, 1, 1,  fptr_dest);
    }
    return e_success;
}
