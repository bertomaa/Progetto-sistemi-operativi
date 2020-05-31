#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "commons.h"

int openWrapper(const char *path, int *fd)
{
    *fd = open(path, O_RDONLY);
    if (*fd < 0)
    {
        fprintf(stderr, "Cannot open file %s.\n", path);
        return 1;
    }
    return 0;
}

//memory allocation
int allocWrapper(int num, int size, void **p)
{
    *p = calloc(num, size);
    if (*p == NULL)
    {
        fprintf(stderr, "Cannot allocate %d * %d byte in memory", num, size);
        return 1;
    }
    addToGC(*p);
    return 0;
}

int reallocWrapper(void** pointer, int size){
    removeFromGC(*pointer);
    *pointer = realloc(*pointer, size);
    if(*pointer == NULL)
    {
        //TODO: qualcosa per sto errore
        fprintf(stderr, "Cannot realloc %d bytes\n", size);
        return 1;
    }
    addToGC(*pointer);
    return 0;
}

//read (pipe)
//If successful, return the non-negative number of bytes actually read, on failure -1
int readWrapper(int read)
{
    if (read == -1)
    {
        fprintf(stderr, "Cannot read the specified file \n");
        return 1;
    }
    return 0;
}

//write (pipe)
//If successful, return the number of bytes actually written, on failure -1
int writeWrapper(int write)
{
    if (write == -1)
    {
        fprintf(stderr, "Cannot write the specified file \n");
        return 1;
    }
    return 0;
}

//close
//If successful, return the number of bytes actually written, on failure -1
int closeWrapper(int close)
{
    if (close == -1)
    {
        fprintf(stderr, "Cannot close the specified file \n");
        return 1;
    }
    return 0;
}