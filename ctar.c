/*
Andrew Mauragis
Systems Programming Project 2
1 Slip day used
*/

#include "ctar.h"

/* helper function for pretty syntax error complaints */
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

/* calcualtes the magic number as specified in the assignment */
int calcMagicNumber(char file_name[256])
{
    int index;
    /* we use uint32s here because sizeof(int) is not guarenteed to be 4, and sizeof(long)
    is 8 on 64 bit systems */
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
        free(buf);
    }
    /*printf("%s's FD: %i\n",archPath,archFD);*/

    /* if we've made it to here without exiting, we have a valid file descriptor for
       the archive */
    lseek(archFD,0,SEEK_SET);

    return archFD;
    
}

/* prints out a list of the contents of the archive */
void listArchive(int archFD)
{
    ssize_t bytesRead;
    hdr_t buf;
    
    printf("Files currently in archive:\n---------------------------\n");
    /* try to read all the headers, until we can't */
    while(0 != read(archFD, &buf, sizeof(hdr_t)))
    {
        if(buf.deleted != 1){
            printf("%s\n",buf.file_name);
        }        
                
        /* move the file descriptor to the next header */
        if(-1 == lseek(archFD,buf.next_header,SEEK_SET))
        {
            fprintf(stderr,"lseek failure\n");
            exit(4);
        }    
    }
    /* put the file descriptor back */
    lseek(archFD,0,SEEK_SET);   
    
}

/* try to append items to an archive, or create the archive if we're pointing at
an empty file */
void appendArchive(int archFD, char* fileList[], int listLen)
{
    /* grab information about the archive, we want to see how big it is to
    determine if this is freshly created or we need to append */
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
        
        /* we iterate through every file in the file list (to be added to archive) */
        for (index = 0; index < listLen; index++)
        {
            currFileFD = open(fileList[index],O_RDONLY);
            if (currFileFD == -1)
            {
                fprintf(stderr,"Could not open '%s' reading.\n",fileList[index]);
                exit(6);
            }

            /* stat file for to fill header */
            if (0 != fstat(currFileFD,statBuffer))
            {
                fprintf(stderr,"couldn't stat file. OS is broken.\n");
                exit(7);
            }

            /* here we create the header struct */
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
            
            /* write the header struct to the archive */
            if (0 >= write(archFD, &fileheader, sizeof(hdr_t)))
            {
                fprintf(stderr, "Could not write to archive.\n");
                exit(8);
            }

            /* now we copy the file into the archive (after the header) in 4k blocks*/
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
        /* put file descriptor back at top of file. we've had a problem here before */
        lseek(archFD,0,SEEK_SET);
    }
    else
    {
        /* this archive already has stuff in it */
        int index;
        int currFileFD;
        

        hdr_t currheader;
        /* we have to go through the existing part of the archive and scan for duplicates */
        while(0 != read(archFD, &currheader, sizeof(hdr_t)))
        {   
            /* this loop iterates through the files, the above while loop iterates through the archive */
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

                /* compare current archive point with current file */
                if(0 == strcmp(fileList[index],currheader.file_name))
                {
                    fprintf(stderr, "Duplicate file '%s' detected.  "
                                    "Please do not add the same file twice.\n",fileList[index]);
                    exit(9);
                }

                /* move the file descriptor to the next header */
                if(-1 == lseek(archFD,currheader.next_header,SEEK_SET))
                {
                    fprintf(stderr,"lseek failure\n");
                    exit(4);
                }
                       
            }
        }
        /* this is just more of the same, once we verify that we don't have any duplicates
        we create the proper header for each file, then copy it block-wise onto the end
        of the archive */
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
        lseek(archFD,0,SEEK_SET);
    }
    free(statBuffer);
}

/* "remove" a file from the archive (by setting the deleted flag) */
int deleteFromArchive(int archFD, char* file)
{
    ssize_t bytesRead;
    hdr_t buf;

    /*check to make sure the file is actually in the archive */
    while(0 != read(archFD, &buf, sizeof(hdr_t)))
    {
        if(0 == strcmp(file,buf.file_name))
        {
            /* once we know we have the file we need, we rewrite the header with the deleted flag */
            buf.deleted = 1;
            lseek(archFD, -1*(sizeof(hdr_t)) ,SEEK_CUR);
            write(archFD, &buf, sizeof(hdr_t));
            /* put the file descriptor back */
            lseek(archFD,0,SEEK_SET);
            return 0;
        }        
                
        /* move the file descriptor to the next header */
        if(-1 == lseek(archFD,buf.next_header,SEEK_SET))
        {
            fprintf(stderr,"lseek failure\n");
            exit(4);
        }    
    }
    /* put the file descriptor back */
    lseek(archFD,0,SEEK_SET);
    return 1;
}

/* extract files from an archive */
void extractArchive(int archFD)
{
    hdr_t buf;
    int fileFD;
    /* iterate through the files */
    while(0 != read(archFD, &buf, sizeof(hdr_t)))
    {
        /* extract relevant data necessary to reconstruct the file */
        char* filename = buf.file_name;
        int filesize = buf.file_size;
        int perms;
        perms |= (buf.p_owner << 6);
        perms |= (buf.p_group << 3);
        perms |= (buf.p_world);
        
        /* if its deleted, skip this iteration (we don't want to extract a deleted file) */
        if(buf.deleted == 1)
        {
            if(-1 == lseek(archFD,buf.next_header,SEEK_SET))
                {
                    fprintf(stderr,"lseek failure\n");
                    exit(4);
                }
                
            continue;   
        }

        /* open the file for writing only, if the if triggers, it exists already */
        if(-1 != (fileFD = open(filename, O_WRONLY)))
        {
            char input[3];
            /* this means that the file does already exist so we must check to overwrite */
            printf("File '%s' already exists.  Do you wish to overwrite it? (y/N) ",buf.file_name);
            fgets(input, 3, stdin);
            
            /* if we get 'y', overwrite, anything else, skip */
            if (0 != (strncmp("y",input,1)))
            {
                /* move the file descriptor to the next header */
                if(-1 == lseek(archFD,buf.next_header,SEEK_SET))
                {
                    fprintf(stderr,"lseek failure\n");
                    exit(4);
                }
                continue;
            }
            
        }
        else
        {   
            /* file does not exist, so create it */
            fileFD = open(filename,O_WRONLY|O_CREAT,perms);
        }
        
        /* at this point we have the file created, now we fill it with crap */

        char filebuffer[4096];
        memset(filebuffer, 0, 4096);
        int bytesRead;
        int failcount = 0;

        /* if we have a file smaller than our copy buffer, just copy the whole file */
        int readsize = ((buf.file_size) < 4096) ? buf.file_size : 4096;

        if(readsize==4096)
        {
            /* this remainingfile nonsense is to prevent us from writing extra into a file
            if it doesn't end up being perfectly divisible by 4k.  it basically allows the last
            block to be smaller */
            int remainingfile = buf.file_size;

            while(0 < (bytesRead = read(archFD, &filebuffer,(remainingfile < readsize) ? remainingfile : readsize)))
            {
                if (0 >= write(fileFD, &filebuffer, bytesRead))
                {
                    fprintf(stderr, "Could not write to file '%s'.\n",buf.file_name);
                    exit(8); 
                  
                }
                remainingfile -= readsize;
            }
        }
        else
        {
            /* we don't need a loop if the file is smaller than one block */
            bytesRead = read(archFD, &filebuffer, readsize );
            if (0 >= write(fileFD, &filebuffer, bytesRead))
            {
                fprintf(stderr, "Could not write to file '%s'.\n",buf.file_name);
                exit(8); 
              
            }
        }

        /* move the file descriptor to the next header */
        
        if(-1 == lseek(archFD,buf.next_header,SEEK_SET))
        {
            fprintf(stderr,"lseek failure\n");
            exit(4);
        }    
        close(fileFD);
    }
    
    /* put the file descriptor back */
    lseek(archFD,0,SEEK_SET);
}