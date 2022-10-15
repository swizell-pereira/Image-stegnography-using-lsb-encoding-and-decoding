#include <stdio.h>
#include<stdlib.h>
#include "encode.h"
#include "types.h"
#include <string.h>
#include"common.h"


/* Check operation type
 * Input: command line argument
 * Output: e_encode or e_decode or e_unsupported
 * Description: To find the operation type base on the command line argument passed
 */
OperationType check_operation_type(char *argv[])
{
    if ( argv[1] != NULL ) // if the 1st argument is not null
    {
	if ( strcmp( *(argv + 1) , "-e" ) == 0) //comapre and check if it is -e
	    return e_encode;
	else if ( strcmp( *(argv + 1) , "-d" ) == 0) //checking if the 1st argument is -d
	    return e_decode;
	else //unsupported operation
	{
	    return e_unsupported;
	}
    }
    else //if arg[1] is empty
    {
	printf("./lsb_steg: Encoding: ./lsb_steg -e <.bmp_file> <.text_file> [output file] \n./lsb_steg:\nDecoding: ./lsb_steg -d <.bmp_file> [output file]");
	exit(1);
    }
}



/* read and validate command line arguments for encoding
 * Input: command line arguments and structure encodeinfo
 * Output: e_success or e_failure
 * Description: To validate the arguments passed with -e
 */
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    //Checking for BMP file
    char arr[2];

    if ( argv[2] != NULL )
    {
	if ( strstr( argv[2] , ".")  != NULL )  //checking if . is provided in the 2nd argument
	{
	    if ( strcmp( strstr( argv[2] , ".") , ".bmp" ) == 0 ) //checking if it is .bmp only
	    {
		encInfo->src_image_fname = argv[2];
		encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r" );
		if( encInfo->fptr_src_image == NULL )		//Error handling
		{
		    perror("fopen");
		    fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);
		    return e_failure;
		}

		fread(arr,2,1,encInfo->fptr_src_image);		//Reading bmp file Image Magic (signature)
		rewind(encInfo->fptr_src_image);

		if ( strcmp(arr,"BM") == 0 )			//Validating bmp file Image Magic (signature)
		{
		    printf("INFO: Correct BMP file\n");
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
    fclose(encInfo->fptr_src_image);				//Closing the file after validation

    //Checking for secret.txt file
    if ( argv[3] != NULL )
    {
	if ( strstr( argv[3] , ".")  != NULL )
	{
	    if ( strcmp( strstr( argv[3] , ".") , ".txt" ) == 0 )
	    {
		encInfo->secret_fname = argv[3];
		encInfo->fptr_secret = fopen(encInfo->secret_fname, "r" );
		if( encInfo->fptr_secret == NULL )		//Error handling
		{
		    perror("fopen");
		    fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);
		    return e_failure;
		}
	    }
	    else
	    {
		fprintf(stderr,"Please pass .txt file for secret file");
		return e_failure;
	    }	
	}
    }
    if ( (argv[3] == NULL) || (strstr( argv[3] , ".") == NULL) )    
    {
	fprintf(stderr,"Please pass secret file name !!");
	return e_failure;
    }
    fclose(encInfo->fptr_secret);

    //Checking if output file name exists or not
    if ( argv[4] != NULL )
    {
	if(strstr(argv[4],".") != NULL)
	{
	    if(strcmp(strstr(argv[4],"."), ".bmp") == 0)
	    {

	encInfo->stego_image_fname = argv[4];	
    }
	}
	if(strstr(argv[4],".") == NULL || strcmp(strstr(argv[4],"."), ".bmp") )
	 {
        fprintf(stderr,"INFO: Please pass .bmp file as output file!!\n");
          return e_failure;
     }
    }
else
	encInfo->stego_image_fname = "steged_img.bmp";  //Default output file name

    return e_success;
}



/* open files
 Input: address of encodeinfo
 * Output: e_success or e_failure
 * Description: opening the required files if they exist else return failure
 */
Status open_files(EncodeInfo *encInfo)
{
    printf("INFO: Opening required files\n");  
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r"); //opening beautiful.bmp
    if (encInfo->fptr_src_image == NULL)    //Error handling
    {
	perror("fopen");
	fprintf(stderr, "INFO: ERROR: Unable to open file %s\n", encInfo->src_image_fname);
	return e_failure;
    }
    else
	printf("INFO: Opened %s\n",encInfo->src_image_fname);

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    if (encInfo->fptr_secret == NULL)	    //Error handling
    {
	perror("fopen");
	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);
	return e_failure;
    }
    else
	printf("INFO: Opened %s\n", encInfo->secret_fname);

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    if (encInfo->fptr_stego_image == NULL)  //Error handling
    {
	perror("fopen");
	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);
	return e_failure;
    }
    
    // No failure return e_success
    return e_success;
}





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





/* Get file size
 * Input: secret file ptr
 * Output: size of secret file
 * Description: in secret file size is obtained by offsetting the ptr to the end
 */


uint get_file_size(FILE *fptr)
{
    fseek(fptr, 0L,SEEK_END);
    return (ftell(fptr));
}


/* check image capacity
 * Input: Image size
 * Output: e_success or e_failure
 * Description: checking if image size has the capacity to perform encoding
 */
Status check_capacity(EncodeInfo *encInfo)
{
    char *extn = strstr( encInfo->secret_fname , "." );
    strcpy ( encInfo->extn_secret_file , extn );

if ( (encInfo->image_capacity) > ( (encInfo->size_secret_file + (strlen(MAGIC_STRING)) + strlen(encInfo->extn_secret_file) + sizeof(int) + sizeof(long) ) * 8 ) ) 
	return e_success;
    else
	return e_failure;
}


/* copy bmp header
 * Input: Image file ptr and output file ptr
 * Output: e_sucess on sucessful copying
 * Description: copy bmp header as it is to output image 
 */


Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
   uint header_size; 
   fseek(fptr_src_image,10L,SEEK_SET);		      //File offset to raster data gives the size of header
   fread(&header_size,sizeof(int),1,fptr_src_image);  //Reading 4 bytes to get size of header
   rewind(fptr_src_image);
   char arr[header_size];
   fread(arr,header_size,1,fptr_src_image);
   fwrite(arr,header_size,1,fptr_dest_image);
   return e_success;
}


/* encode magic string
 * Input: magic string and address of encodeinfo
 * Output: e_success on succesfull coping
 * Description: taking one bit from the magic string and encoding it with image data
 */

Status encode_magic_string( char *magic_string, EncodeInfo *encInfo)
{
    encode_data_to_image( magic_string , strlen(magic_string) , encInfo->fptr_src_image , encInfo->fptr_stego_image );
    return e_success;
}


/* encode data to image
 * Input: string of bytes , no of bytes, image and secret file pts
 * Output: e_success
 * Description: read 8 bytes at a time pass it to encode byte to lsb storing the bits accordingly 
 */
Status encode_data_to_image( char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image)
{
    int i;
    char arr[8];
    for ( i = 0; i < size; i++ )
    {
	fread(arr,8,1,fptr_src_image);
	encode_byte_to_lsb( *(data + i) , arr );
	fwrite(arr,8,1,fptr_stego_image);
    }

}

/* encode byte to lsb
 * Input: a charactor , buffer of 8 byte from data
 * Output: e_success
 * Description: storing each bit of charactor to the image data 
 */

Status encode_byte_to_lsb(char data, char *image_buffer)
{
    unsigned char bit , mask = 1;
    int i;
    for ( i = 0 ; i < 8; i++ )
    {
	bit = ( (data >> (7 - i)) & (mask) );
	*(image_buffer + i ) = ( *(image_buffer + i) & (~mask) ) | bit;
    }
    
}


/* encode secret file extension size
 * Input: size, add of encodeinfo
 * Output: e_status
 * Description: to encode extension size
 */

Status encode_secret_file_extn_size(int file_extn_size, EncodeInfo *encInfo)
{
    int i;
    char arr[4];
    for ( i = 0; i < 4; i++ )
    {
	arr[i] = file_extn_size & 0xff;
	file_extn_size >>= 8;

    }
    encode_data_to_image( arr, sizeof(int), encInfo->fptr_src_image , encInfo->fptr_stego_image );
    return e_success;
}

/* encode secret file extension
 * Input: string, address of encodeinfo
 * Output: e_success
 * Description: to encode secret file extension
 */
Status encode_secret_file_extn( char *file_extn, EncodeInfo *encInfo)
{
    encode_data_to_image( file_extn, strlen(file_extn),encInfo->fptr_src_image, encInfo->fptr_stego_image );
    return e_success;
}


/* encode secret file size
 * Input: size , address of encodeinfo
 * Output: e_success
 * Description: to encode secret file size
 */
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{   
    long x = file_size, i;
    char arr[8];
    for ( i = 0; i < 8; i++ )
    {
	arr[i] = x & 0xff;
	x = x >> 8;
    }
    encode_data_to_image( arr, sizeof(long), encInfo->fptr_src_image , encInfo->fptr_stego_image );
    return e_success;
}

/* encode secret file data
 * Input: add of encodeinfo
 * Output: e_success
 * Description: to encode secret file data
 */

Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char secret_data[encInfo->size_secret_file];
    fseek(encInfo->fptr_secret,0L,SEEK_SET);
    fread(secret_data,encInfo->size_secret_file,1,encInfo->fptr_secret);
    encode_data_to_image( secret_data,encInfo->size_secret_file,encInfo->fptr_src_image,encInfo->fptr_stego_image);
    return e_success;
}

/* copy remaining data
 * Input: image ptr and file ptr size
 * Output: e_success
 * Description: encode remaining data
 */
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest , unsigned long int size)
{
    char arr[size];
    fread(arr,size,1,fptr_src);
    fwrite(arr,size,1,fptr_dest);
    return e_success;
}




Status do_encoding(EncodeInfo *encInfo)
{
    uint size;
    //Test open_files
    if (open_files(encInfo) == e_failure)
    {
	printf("INFO: ERROR: %s function failed\n", "open_files" );
	return 1;
    }
    else
    {
	printf("INFO: SUCCESS: %s function completed\n", "open_files" );
    }

encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);
    printf("INFO: Image size = %u\n", encInfo->image_capacity) ;

    printf("INFO: ## Encoding Procedure started ##\n");
    
    //Checking size of secret file
    printf("INFO: Checking for %s size\n", encInfo->secret_fname);
    if ( (encInfo->size_secret_file = get_file_size(encInfo->fptr_secret))  > 0 )
	printf("INFO: Done not empty\n");
    else
    {
	printf("INFO: Error: EMPTY\n");
	return e_failure;
    }



    printf("INFO: Checking for %s capacity to handle %s\n", encInfo->src_image_fname, encInfo->secret_fname);
    if(check_capacity(encInfo) == e_success)
	printf("INFO: Done found ok\n");
    else
    {
	printf("INFO: %s deosnt have the capacity to handle %s\n", encInfo->src_image_fname, encInfo->secret_fname);
	return e_failure;
    }

    //Opening output file with default name
    if( strcmp( encInfo->stego_image_fname, "steged_img.bmp") == 0 )
	printf("INFO: Output File not mentioned. Creating steged_img.bmp as default\n");

    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w+");
    if (encInfo->fptr_stego_image == NULL)
    {
	perror("INFO: fopen");
	fprintf(stderr, "INFO: ERROR: Unable to open file %s\n", encInfo->stego_image_fname);
    }

    printf("INFO: Copying image header\n");
    if ( copy_bmp_header( encInfo->fptr_src_image , encInfo->fptr_stego_image ) == e_success)
	printf("INFO: Done\n");

    printf("INFO: Encoding magic string signature\n");
    if ( encode_magic_string( MAGIC_STRING , encInfo ) == e_success)
	printf("INFO: Done\n");
    else
    {
	printf("INFO: Not done\n");
	return e_failure;
    }

    printf("INFO: Encoding %s File Extension Size\n", encInfo->secret_fname);
    if ( encode_secret_file_extn_size( strlen(encInfo->extn_secret_file) ,encInfo ) == e_success)
	printf("INFO: Done\n");
    else
    {
	printf("INFO: Not done\n");
	return e_failure;
    }

    printf("INFO: Encoding %s File Extension\n", encInfo->secret_fname);
    if ( encode_secret_file_extn( encInfo->extn_secret_file, encInfo ) == e_success)
	printf("INFO: Done\n");
    else
    {
	printf("INFO: Not done\n");
	return e_failure;
    }

    printf("INFO: Encoding %s File Size\n", encInfo->secret_fname);
    if ( encode_secret_file_size( encInfo->size_secret_file, encInfo ) == e_success)
	printf("INFO: Done\n");
    else
    {
	printf("INFO: Not done\n");
	return e_failure;
    }

    printf("INFO: Encoding %s File Data\n", encInfo->secret_fname);
    if ( encode_secret_file_data( encInfo ) == e_success)
	printf("INFO: Done\n");
    else
    {
	printf("INFO: Not done\n");
	return e_failure;
    }

    printf("INFO: Copying left over data\n");

    unsigned long int remaining_img_size = ( (encInfo->image_capacity + 55) - ftell(encInfo->fptr_src_image) );

    if ( copy_remaining_img_data( encInfo->fptr_src_image,encInfo->fptr_stego_image,remaining_img_size) == e_success)
	printf("INFO: Done\n");
    else
    {
	printf("INFO: Not done\n");
	return e_failure;
    }
}






