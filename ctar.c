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
