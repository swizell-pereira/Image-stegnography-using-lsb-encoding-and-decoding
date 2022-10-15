#include <stdio.h>
#include <stdlib.h>
#include "encode.h"
#include "decode.h"
#include "types.h"

int main(int argc, char **argv)
{

    EncodeInfo encInfo;
    DecodeInfo decInfo;
    uint img_size;
    if(check_operation_type( argv ) == e_encode )
    {
	printf("INFO: You have selected encoding\n");
	//Reading and validating arguments
	if( read_and_validate_encode_args( argv, &encInfo) == e_failure )
	    return -1;
	if ( do_encoding(&encInfo) == e_failure )
	    return -1;
    }
    else if(check_operation_type( argv ) == e_decode )
    {
	printf("INFO: You have selected decoding\n");
	if( read_and_validate_decode_args( argv, &decInfo) == e_failure )
	    return -1;
	if ( do_decoding(&decInfo) == e_failure )
	    return -1;

    }
    else
    {
	printf("INFO: Error: Unsupported operation\n");
	return -1;
    }
    return 0;
}
