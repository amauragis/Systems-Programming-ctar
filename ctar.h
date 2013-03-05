#ifndef ctar_h
#define ctar_h

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <stdint.h>

typedef struct
{
    int magic_number;       /* CS3411 tar file */
    int next_header;        /* offset of next header relative to beginning of file */
    int file_size;          /* file size in bytes */
    char deleted;           /* if deleted, = 1, otherwise 0 */
    char p_owner;           
    char p_group;
    char p_world;
    char file_name[256];
} hdr_t;

typedef struct stat stat_t;

void syntaxError(char* argv[]);
void notValidArchive(char* archPath);
int openArchive(char* archPath);
int calcMagicNumber(char file_name[256]);
char* listArchive(int archFD);

#endif