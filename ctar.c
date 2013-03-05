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
            fprintf(stderr, "Cannot create archive '%s'",archPath);
            exit(3);
        } 

    }else
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
        memset(file_name,0,256);
        strcpy(file_name,archPath);
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
    /* try to read first header */
    while(0 != read(archFD, buf, sizeof(hdr_t)))
    {
        strLen = strlen(buf->file_name)+1;
        if (strLen+listLen >= listSize)
        {
            fileList = realloc(fileList,listSize+256)
        }
        memcpy(fileList+listLen, buf->file_name,strLen);
        listLen += strLen;
        memcpy(fileList+listLen, "\n",1);
        listlen += 1;

        if(-1 == lseek(archFD,buf->next_header,SEEK_SET))
        {
            fprintf(stderr,"lseek Failure");
            exit(4);
        }    
    }
    lseek(archFD,0,SEEK_SET);
    return fileList;
}