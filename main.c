#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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
    unsigned char aflag = 0;
    unsigned char dflag = 0;
    unsigned char eflag = 0;
    unsigned char lflag = 0;
    unsigned char errflag = 0;

    printf("argc: %i\n",argc);
    printf("optarg: %s\n",optarg);
    if (argc < 3) syntaxError(argv);
    opt = getopt(argc, argv, "a:d:e:l:");
        switch (opt) {
        case 'a':
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
    
    if (errflag) syntaxError(argv);
    printf("archiveName=%s\n optind=%d\n", archiveName, optind);

    if (optind >= argc) {
        fprintf(stderr, "Expected argument after options\n");
        exit(1);
    }

    printf("name argument = %s\n", argv[optind]);


    /* Other code omitted */

    exit(0);
}