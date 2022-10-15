#include <stdio.h>
#include<stdlib.h>
#include<string.h>
#include "decode.h"
#include "types.h"
#include "common.h"

/* Function Definitions */
int secret_fname_flag;

Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    //Checking BMP file
    char arr[2];    //Temporary array for storing first 2 bytes of source image

    if ( argv[2] != NULL )
    {
	if ( strstr( argv[2] , ".")  != NULL )
	{
	    if ( strcmp( strstr( argv[2] , ".") , ".bmp" ) == 0 )
	    {
		decInfo->steg_image_fname = argv[2];
		decInfo->fptr_steg_image = fopen(decInfo->steg_image_fname, "r" );
		if( decInfo->fptr_steg_image == NULL )			//Error handling
		{
		    perror("fopen");
		    fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->steg_image_fname);
		    return e_failure;
		}

		if( fread(arr,1,2,decInfo->fptr_steg_image) != 2 )	//Reading bmp file Image Magic (signature)
		{
		    fprintf(stderr, "ERROR: Unable to read file %s\n", decInfo->steg_image_fname);
		    return e_failure;
		}
		    
		rewind(decInfo->fptr_steg_image);			//Resetting file pointer to start of file

		if ( strcmp(arr,"BM") == 0 )				//Validating bmp file Image Magic (signature)
		{
		    printf("INFO: ## Decoding Procedure Started ##\n");
		    printf("INFO: Opening required files\n");
		    printf("INFO: Correct BMP file\n");
		    printf("INFO: Opened %s\n", decInfo->steg_image_fname);
		}
		else
		{
		    fprintf(stderr,"INFO: Error: Not a proper BMP file\n");
		    return e_failure;
		}
	    }
	}
    }
    else
    {
	fprintf(stderr,"INFO: Error: 2nd argument empty !! Please provide a .bmp file");
	return e_failure;
    }
    if ( (strstr( argv[2] , ".") == NULL) || (strcmp( strstr( argv[2] , ".") , ".bmp" )) )    
    {
	fprintf(stderr,"INFO: Please pass .bmp file !!");
	return e_failure;
    }

    //Checking name of output file
    if ( argv[3] != NULL )
    {
	strcpy(decInfo->secret_fname,argv[3]);
	secret_fname_flag = 1;			//Setting flag to 1 if output file name is provided by the user
    }

return e_success;
}

uint get_bmp_header_size(FILE *fptr_image)
{
    uint size;
    fseek(fptr_image,10L,SEEK_SET);		//Updating File pointer to File offset to Raster Data location
    fread(&size,4,1,fptr_image);		//Reading 4 bytes to get bmp header size info
    rewind(fptr_image);				
    return size;
}

Status do_decoding(DecodeInfo *decInfo)
{
    decInfo->header_size = get_bmp_header_size(decInfo->fptr_steg_image);   //Getting file header size
    fseek(decInfo->fptr_steg_image,decInfo->header_size,SEEK_SET);	    //Updating File pointer to RGB data of the file

    printf("INFO: Decoding magic string signature\n");
    if ( decode_magic_string(decInfo) == e_failure )
    return e_failure;
    printf("INFO: Done\n");

    printf("INFO: Decoding Output File Extension Size\n");
    if ( decode_secret_file_extn_size(decInfo) == e_failure )
    return e_failure;
    printf("INFO: Done\n");

    printf("INFO: Decoding Output File Extension\n");
    if ( decode_secret_file_extn(decInfo) == e_failure )
    return e_failure;
    printf("INFO: Done\n");

    if (secret_fname_flag == 1 )
    {
	strcat(decInfo->secret_fname,decInfo->extn_secret_file);
	printf("INFO: Creating %s as output file\n", decInfo->secret_fname);
    }
    else
    {
	printf("INFO: Creating decoded%s as default output file\n", decInfo->extn_secret_file);
	strcpy(decInfo->secret_fname,"decode");
	strcat(decInfo->secret_fname,decInfo->extn_secret_file);
	//printf("%s" ,decInfo->secret_fname);
    }
    decInfo->fptr_secret = fopen(decInfo->secret_fname,"w+");
    if ( decInfo->fptr_secret == NULL )
    {
	printf("INFO: Error in opening %s\n",decInfo->secret_fname);
	return e_failure;
    }
    printf("INFO: Opened %s\n",decInfo->secret_fname);
    printf("INFO: Opened all required files\n");

    printf("INFO: Decoding file size\n");
    if ( decode_secret_file_size(decInfo) == e_failure )
    return e_failure;
    printf("INFO: Done\n");

    printf("INFO: Decoding file data\n");
    if ( decode_secret_file_data(decInfo) == e_failure )
    printf("INFO: Done\n");
    printf("INFO ## Decoding Done Successfully ##\n");
}

Status decode_magic_string(DecodeInfo *decInfo)
{
    decode_data_from_image(strlen(MAGIC_STRING), decInfo);

    printf("%s", decInfo->op_data);
    if ( strcmp(decInfo->op_data, MAGIC_STRING) != 0 )		    //Checking existence of Magic string in the bmp file
    {
	printf("INFO: Error: Magic string does not match\n");
	return e_failure;
    }
    printf("%s", decInfo->op_data);
}

Status decode_secret_file_extn_size(DecodeInfo *decInfo)
{
    int i,a = 0;
    decode_data_from_image(sizeof(int), decInfo);
    //decInfo->size_secret_file_extn = decInfo->op_data[0];

    for( i = 0; i < 4; i++ )					    //Converting bits stored in character array into integer
    {
	a = a | (decInfo->op_data[i]) << (i * 8);
    }

    decInfo->size_secret_file_extn = a;
    //printf("%u", decInfo->size_secret_file_extn);
} 

Status decode_secret_file_extn(DecodeInfo *decInfo)
{
    decode_data_from_image(decInfo->size_secret_file_extn,decInfo);
    strcpy(decInfo->extn_secret_file,decInfo->op_data);
    //printf("%s", decInfo->extn_secret_file);
}

Status decode_secret_file_size(DecodeInfo *decInfo)		   {
    long a = 0; int i;
    decode_data_from_image(sizeof(long), decInfo);
    //decInfo->size_secret_file = decInfo->op_data[0];

    for( i = 0; i < 8; i++ )					    //Converting bits stored in character array into integer
    {
	a = a | (decInfo->op_data[i]) << (i * 8);
    }

    decInfo->size_secret_file = a;
    //printf("%ld" , decInfo->size_secret_file);
}

Status decode_secret_file_data(DecodeInfo *decInfo)
{
    decode_data_from_image(decInfo->size_secret_file,decInfo);
    //printf("%s", decInfo->op_data);
    fwrite(decInfo->op_data,decInfo->size_secret_file,1,decInfo->fptr_secret);
}

Status decode_data_from_image(int size, DecodeInfo *decInfo)
{
    unsigned char a[8] ;
    int i;

    for( i = 0; i < size; i++ )
    {
	fread( a,8,1,decInfo->fptr_steg_image );
	*(decInfo->op_data + i) = decode_byte_from_lsb(a);
    }
    *(decInfo->op_data + i) = '\0'; 
    //printf("%s" , decInfo->op_data);
}

char decode_byte_from_lsb(char *image_buffer)
{
    unsigned char bit, mask = 1,arr[8],ch;
    unsigned int i,j,sum = 0;

    for ( i = 0; i < 8; i++ )
    {
	*(arr + i) = *(image_buffer + i) & mask;
    }

    j = 128;
    for ( i = 0; i < 8; i++ )
    {
	sum = sum + (arr[i] * j);
	j = j/2;
    }
    return ((char)sum);
}

