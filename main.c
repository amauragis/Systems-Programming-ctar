/*
Andrew Mauragis
Systems Programming Project 2
1 Slip day used
*/
#include "ctar.h"


int main(int argc, char* argv[])
{

    int opt;
    char* archiveName;
    extern int optind;
    extern char* optarg;
    
    /* check for appropriate arguments */
    opt = getopt(argc, argv, "a:d:e:l:");
        switch (opt)
        {
            case 'a':
            {
            /* This is append mode, it takes the format 
               ./ctar -a <archive file name> file1 file2 ... filen */   
                if (argc < 4) syntaxError(argv);

                int archiveFD; 
                int argIndex;

                /* go through and check to make sure no files share a name here*/
                int i;
                for (i = 3; i < argc; i++)
                {
                    int j;
                    for (j = i + 1; j < argc; j++)
                    {                        
                        if(0 == strcmp(argv[i],argv[j]))
                        {
                            fprintf(stderr,"Duplicate file '%s' detected.  "
                                "Please do not add the same file twice.\n",argv[i]);
                            exit(9);
                        }
                    }
                }
               
                /* XXX: -3 used because we have <invocation> -a <archive> <file1> <file2> */
                int multifile = argc - 3;
                char* filelist[multifile];
                
                for(argIndex = 3; argIndex < argc; argIndex++)
                {
                    filelist[argIndex-3] = argv[argIndex];
                    
                }                
                archiveName = optarg;
                archiveFD = openArchive(archiveName, O_RDWR);
                appendArchive(archiveFD, filelist, multifile);

                break;
            }
            case 'd':
            {
                /* delete mode returns 1 if we can't delete the file, allowing the error
                reporting out here. */
                if (argc != 4) syntaxError(argv);
                
                char* file = argv[3];
                int archiveFD;
                archiveName = optarg;
                archiveFD = openArchive(archiveName, O_RDWR);
                if (0 != deleteFromArchive(archiveFD, file))
                {
                    printf("Cannot delete file '%s' from archive.\n",file);
                }
                break;
            }
            case 'e':
            {
                /* pretty simple. calls extract function */
                if (argc != 3) syntaxError(argv);
                int archiveFD;            
                archiveName = optarg;
                archiveFD = openArchive(archiveName,O_RDONLY);
                extractArchive(archiveFD);

                break;
            }
            case 'l':
            {
                /* list function */
                if (argc != 3) syntaxError(argv);
                
                archiveName = optarg;
                int archiveFD = openArchive(archiveName, O_RDONLY);
                listArchive(archiveFD);
                break;
            }
            default:
            { /* '?' */
                syntaxError(argv);    
                /* CODE BELOW HERE IS DEAD FOR TESTING        
                int magicnumber;
                char file_name[256] = "chelseasucks.c";
                magicnumber = calcMagicNumber(file_name);
                printf("%i\n",magicnumber); */
                
            }   
        }
    
   
    if (optind > argc) {
        /* more command line checking */
        fprintf(stderr, "Expected argument after options\n");
        exit(1);
    }

    exit(0);
}
