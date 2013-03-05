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
    uint32_t* wordarray = (uint32_t*)file_name;
    int magicnum = 0;
    for (index = 0; index < 64; index++)
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

    /* if we fail to open the file, try to create it */
    if (archFD == -1)
    {
        archFD = open(archPath, O_RDWR|O_CREAT, 0644);
        
        /* if we still can't create it, something is wrong */
        if (archFD == -1)
        {
            fprintf(stderr, "Cannot create archive '%s'\n",archPath);
            exit(3);
        } 

    }
    else
    /* otherwise, we check to make sure its a valid archive */
    {
        ssize_t bytesRead;
        hdr_t* buf = malloc(sizeof(hdr_t));
        
        /* try to read the first header */
        bytesRead = read(archFD, buf, sizeof(hdr_t));
        
        /* if we couldn't read the full header of bytes, clearly its not an archive */
        if (bytesRead != sizeof(hdr_t))
        {
            notValidArchive(archPath);
        }

        /* now we compare the magic number listed in the archive to the one we
           calculate ourselves.  If they don't match, it's not an archive */
        int magicnumber;
        char file_name[256];
        memcpy(file_name,buf->file_name,256);        
        magicnumber = calcMagicNumber(file_name);
        /*printf("magic number: %i\n",magicnumber);*/
        if(buf->magic_number != magicnumber)
        {
            free(buf);
            notValidArchive(archPath);  
        } 
    }
    /*printf("%s's FD: %i\n",archPath,archFD);*/

    /* if we've made it to here without exiting, we have a valid file descriptor for
       the archive */
    return archFD;
    
}

char* listArchive(int archFD)
{
    ssize_t bytesRead;
    char* fileList;
    size_t listSize = 0;
    size_t listLen = 0;
    hdr_t* buf = malloc(sizeof(hdr_t));
    /* try to read all the headers, until we can't */
    while(0 != read(archFD, buf, sizeof(hdr_t)))
    {
        /* grab length of string, add space for null terminator */
        int strLen = strlen(buf->file_name)+1;

        /* if we've run out of space for our list, we add 256 more bytes */
        if (strLen+listLen >= listSize) fileList = realloc(fileList,listSize+256);
        
        /* copy the string to the list, then copy "\n" in after it */
        memcpy(fileList+listLen, buf->file_name,strLen);
        listLen += strLen;
        memcpy(fileList+listLen, "\n",1);
        listLen += 1;

        /* move the file descriptor to the next header */
        if(-1 == lseek(archFD,buf->next_header,SEEK_SET))
        {
            fprintf(stderr,"lseek Failure\n");
            exit(4);
        }    
    }
    /* put the file descriptor back */
    lseek(archFD,0,SEEK_SET);
    return fileList;
}

void appendArchive(int archFD, char* fileList[], int listLen)
{
    stat_t* statBuffer = malloc(sizeof(stat_t));
    if (0 != fstat(archFD,statBuffer))
    {
        fprintf(stderr,"couldn't stat archive. OS is broken.\n");
        exit(5);
    }
    if(statBuffer->st_size == 0)
    {
        /* This is a freshly created archive */
        int index;
        int currFileFD;
        int currFileOwnPerm;
        int currFileGroupPerm;
        int currFileWorldPerm;
        int currFilePerm;

        /* go through and check to make sure no files share a name */
        for (index = 0; index < listLen; index++)
        {
            currFileFD = open(fileList[index],O_RDONLY);
            if (currFileFD == -1)
            {
                fprintf(stderr,"Could not open '%s' reading.\n",fileList[index]);
                exit(6);
            }
            if (0 != fstat(currFileFD,statBuffer))
            {
                fprintf(stderr,"couldn't stat file. OS is broken.\n");
                exit(7);
            }
            currFileOwnPerm = (statBuffer->st_mode) & S_IRWXU;
            currFileGroupPerm = (statBuffer->st_mode) & S_IRWXG;
            currFileWorldPerm = (statBuffer->st_mode) & S_IRWXO;
            currFilePerm = currFileOwnPerm | currFileGroupPerm | currFileWorldPerm;
            printf("Permissions for %s: %o\n",fileList[index],currFilePerm);

        }
    }
    else
    {
        /* this archive already has stuff in it */

    }
}
