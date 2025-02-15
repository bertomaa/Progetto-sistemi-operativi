#ifndef __COMMONS_H__
#define __COMMONS_H__

#define MAX_COMMAND_LEN 2048
#define PRE_FILES_ARGS 3
#define MAX_PATH_LEN 1024
#define INITIAL_CONFIG_SIZE 1024
#define READ 0
#define WRITE 1
#define ASCII_CHARACTERS 256
#define MAX_PIPE_CHARACTERS 8128
#define FATHER 0
#define CHILD 1

typedef struct garbageCollector
{
    int dim;
    int garbageCount;
    void **garbage;
} garbageCollector;

typedef struct pairIntInt
{
    int first;
    int second;
} pairIntInt;

extern int isCollectingGarbage;
extern garbageCollector gc;

//void printProgressBar(int now, int tot);

int initGC();

void addToGC(void *garbage);

void removeFromGC(void *p);

char *getLine();

void removeFromGCAndFree(void *p);

int isFileNameAcceptable(char* fileName);

void collectGarbage();

void addDoubleQuotes(char *buffer, char *path);

char *removeDoubleQuotes(char *buffer, char *path);

char *getCommandOutput(const char *cmd, int bufferSizeInBytes);

int getPipeIndex(int index, int type);

char *splitString(char *buffer, char **str, char delimiter);

int getDigits(int n);

int does1StringMatch2(char *s1, char *s2);

void fatalErrorHandler(char *message, int errorCode);

char* splitStringWithQuotes(char *buffer, char **str, char delimiter);

int getFilesCountInPath(char * path);

void merge(pairIntInt arr[], int l, int m, int r);

void mergeSort(pairIntInt arr[], int l, int r);

void reverse(pairIntInt arr[], int n);

pairIntInt makePair(int f, int s);


#endif