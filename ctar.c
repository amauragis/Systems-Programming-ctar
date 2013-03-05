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

void notValidArchive(char* archPath)
{
    fprintf(stderr, "'%s' is not a valid archive file.\n",archPath);
    exit(2);
}

int calcMagicNumber(char file_name[256])
{
    int index;
    long* wordarray = (long*)file_name;
    int numwords = sizeof(file_name)/sizeof(long);
    int magicnum = 0;
    for (index = 0; index < numwords; index++)
    {
        magicnum = magicnum ^ wordarray[index];
    }
    return magicnum;
}

/*
Tries to open the archive specified as archPath.  If an archive does not exist,
create one.  If the file specified is not an archive, panic.
*/
int openArchive(char* archPath)
{
    int archFD;
    archFD = open(archPath, O_RDWR);
    if (archFD == -1)
    {
        puts("creating new file");
        archFD = open(archPath, O_RDWR|O_CREAT, 0644);
    }else
    {
        ssize_t bytesRead;
        stat_t* buf = malloc(sizeof(hdr_t));
        bytesRead = read(archFD, buf, sizeof(hdr_t));
        if (bytesRead != sizeof(hdr_t))
        {
            notValidArchive(archPath);
        }
        int magicnumber;
        char file_name[256];
        memset(file_name,0,256);
        strcpy(file_name,archPath);
        magicnumber = calcMagicNumber(file_name);
        printf("magic number: %i\n",magicnumber);
    }
    printf("%s's FD: %i\n",archPath,archFD);
    
}