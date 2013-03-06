#include "ctar.h"

void syntaxError(char* argv[])
{
    fprintf(stderr, "Usage:\n"
                    "%s -a <archive> <file1> <file2> ... <filen>\n"
                    "%s -d <archive> <file>\n"
                    "%s -e <archive>\n"
                    "%s -l <archive>\n",
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
int openArchive(char* archPath, int flags)
{
    int archFD;
    archFD = open(archPath, flags);

    /* if we fail to open the file, try to create it */
    if (archFD == -1)
    {
        archFD = open(archPath, flags|O_CREAT, 0644);
        
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
              
        magicnumber = calcMagicNumber(buf->file_name);
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
    lseek(archFD,0,SEEK_SET);
    return archFD;
    
}

void listArchive(int archFD)
{
    ssize_t bytesRead;
    char* fileList = NULL;
    size_t listSize = 0;
    size_t listLen = 0;
    hdr_t* buf = malloc(sizeof(hdr_t));
    
    printf("Files currently in archive:\n---------------------------\n");
    /* try to read all the headers, until we can't */
    while(0 != read(archFD, buf, sizeof(hdr_t)))
    {
        /* grab length of string, add space for null terminator */
        int strLen = strlen(buf->file_name)+1;

        /* if we've run out of space for our list, we add 256 more bytes */
        if (strLen+listLen >= listSize)
        {
            fileList = realloc(fileList,listSize+256);    
        }
        
        /* copy the string to the list, then copy "\n" in after it */
        if(buf->deleted != 1){
            printf("%s\n",buf->file_name);
        }        
                
        /* move the file descriptor to the next header */
        if(-1 == lseek(archFD,buf->next_header,SEEK_SET))
        {
            fprintf(stderr,"lseek Failure\n");
            exit(4);
        }    
    }
    /* put the file descriptor back */
    lseek(archFD,0,SEEK_SET);
    free(buf);
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
        
        /* go through and check to make sure no files share a name here*/


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

            hdr_t fileheader;

            char file_name[256];
            memset(file_name, 0, 256);
            strcpy(file_name,fileList[index]);  
            memcpy(fileheader.file_name,file_name,256);
            fileheader.magic_number = calcMagicNumber(file_name);
            fileheader.file_size = statBuffer->st_size;
            fileheader.deleted = 0;
            fileheader.p_owner = ((statBuffer->st_mode) & S_IRWXU) >> 6;
            fileheader.p_group = ((statBuffer->st_mode) & S_IRWXG) >> 3;
            fileheader.p_world = (statBuffer->st_mode) & S_IRWXO;
            off_t currloc = lseek(archFD, 0, SEEK_CUR);
            fileheader.next_header = currloc + sizeof(hdr_t) + fileheader.file_size;
            printf("sizeof(hdr_t): %i\n",sizeof(hdr_t));
            printf("file_size: %i\n",fileheader.file_size);
            printf("File: %s\n",fileList[index]);
            printf("Current Location: %i\n",currloc);
            printf("Next Location: %i\n",fileheader.next_header);
            puts("");

            if (0 >= write(archFD, &fileheader, sizeof(hdr_t)))
            {
                fprintf(stderr, "Could not write to archive.\n");
                exit(8);
            }
            int bytesRead;
            char buf[4096];
            while((bytesRead = read(currFileFD, &buf, 4096)) > 0)
            {
                if (0 >= write(archFD, &buf, bytesRead))
                {
                    fprintf(stderr, "Could not write to archive.\n");
                    exit(8);
                }
            }
            close(currFileFD);

        }
    }
    else
    {
        /* this archive already has stuff in it */

    }
}
