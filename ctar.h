#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct
{
	int magic_number; 		/* CS3411 tar file */
	int next_header;  		/* offset of next header relative to beginning of file */
	int file_size;			/* file size in bytes */
	char deleted;			/* if deleted, = 1, otherwise 0 */
	char p_owner;			
	char p_group;
	char p_world;
	char file_name[256];
} hdr;

void syntaxError(char* argv[]);