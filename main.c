#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "ctar.h"

void syntaxError(char* argv[])
{
	fprintf(stderr, "Usage: \t %s -a <archive file name> file1 file2 ... filen\n"
                    "\t %s -d <archive file name> file\n"
            		"\t %s -e <archive file name>\n"
            		"\t %s -l <archive file name>\n",
                    argv[0],argv[0],argv[0],argv[0]);
    exit(1);
}

int main(int argc, char* argv[])
{

    int opt;
    char* archiveName;
    extern int optind;
    extern char* optarg;
   	unsigned char multifile = 0;

    printf("argc: %i\n",argc);
    
    /*if (argc < 3) syntaxError(argv);*/
    opt = getopt(argc, argv, "a:d:e:l:");
    printf("optarg: %s\n",optarg);
        switch (opt) {
        case 'a':
        	multifile = 1;
        	puts("append mode");
        	archiveName = optarg;
            break;
        case 'd':
        	puts("delete mode");
        	archiveName = optarg;
            break;
        case 'e':
        	puts("extract mode");
        	archiveName = optarg;
            break;
        case 'l':
        	puts("list mode");
        	archiveName = optarg;
            break;
        default: /* '?' */
            syntaxError(argv);           
        }
    
   
    if (optind >= argc) {
        fprintf(stderr, "Expected argument after options\n");
        exit(1);
    }

    printf("non-opt argument = %s\n", argv[optind]);


    /* Other code omitted */

    exit(0);
}