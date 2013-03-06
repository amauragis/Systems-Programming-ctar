#include "ctar.h"


int main(int argc, char* argv[])
{

    int opt;
    char* archiveName;
    extern int optind;
    extern char* optarg;

    /* contains the number of expected file arguments. */
    short int multifile;

    /*printf("argc: %i\n",argc);*/
    
    opt = getopt(argc, argv, "a:d:e:l:");
    /*printf("optarg: %s\n",optarg);*/
        switch (opt)
        {
            case 'a':
            {
            /* This is append mode, it takes the format 
               ./ctar -a <archive file name> file1 file2 ... filen */   
                if (argc < 4) syntaxError(argv);

                int archiveFD; 
                int argIndex;
               
                /* XXX: -3 used because we have <invocation> -a <archive> <file1> <file2> */
                multifile = argc - 3;
                char* filelist[multifile];
                printf("Files: ");
                for(argIndex = 3; argIndex < argc; argIndex++)
                {
                    filelist[argIndex-3] = argv[argIndex];
                    printf("%s ",argv[argIndex]);
                }
                printf("\n");

                puts("append mode");
                archiveName = optarg;
                archiveFD = openArchive(archiveName, O_RDWR);
                appendArchive(archiveFD, filelist, multifile);

                break;
            }
            case 'd':
            {
                if (argc != 4) syntaxError(argv);
                multifile = 1;
                puts("delete mode");
                /*file = argv[3]*/
                archiveName = optarg;
                break;
            }
            case 'e':
            {
                if (argc != 3) syntaxError(argv);
                multifile = 0;
                puts("extract mode");
                archiveName = optarg;
                break;
            }
            case 'l':
            {
                if (argc != 3) syntaxError(argv);
                           
                puts("list mode");
                archiveName = optarg;
                int archiveFD = openArchive(archiveName, O_RDONLY);
                listArchive(archiveFD);
                break;
            }
            default:
            { /* '?' */
                syntaxError(argv);           
                int magicnumber;
                char file_name[256] = "chelseasucks.c";
                magicnumber = calcMagicNumber(file_name);
                printf("%i\n",magicnumber);
                
            }   
        }
    
   
    if (optind > argc) {
        fprintf(stderr, "Expected argument after options\n");
        exit(1);
    }

    /* printf("non-opt argument = %s\n", argv[optind]); */


    exit(0);
}
