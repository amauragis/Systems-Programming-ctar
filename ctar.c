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
        
    }
    printf("%s's FD: %i\n",archPath,archFD);
    
}