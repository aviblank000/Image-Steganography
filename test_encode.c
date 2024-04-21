#include <stdio.h>
#include "encode.h"
#include "decode.h"
#include "types.h"

int main(int argc, char *argv[])
{
    EncodeInfo encInfo;
    DecodeInfo decInfo;

    if (argc < 3)
    {
	printf("Enter valid number of CLA\n");
	return -1;
    }
    
    if (check_operation_type(argv) == e_encode)
    {
	 if (read_and_validate_encode_args(argv, &encInfo) == e_failure)
	 {
	     printf("Please enter valid file name\n");
	     return -1;
	 }
	 do_encoding(&encInfo);
    }
    else if (check_operation_type(argv) == e_decode)
    {
	if (read_and_validate_decode_args(argv, &decInfo) == d_failure)
	{
	     printf("Please enter valid file name\n");
	     return -1;
	}
	do_decoding(&decInfo);
    }

    return 0;
}
