#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <math.h>
#include <tgmath.h>
#include "commons.h"
#include "stats.h"
#include "wrapper.h"

void initStats(stats *stat, int _fileID)
{
    stat->fileID = _fileID;
    int i;
    for (i = 0; i < ASCII_CHARACTERS; i++)
    {
        stat->frequencies[i] = 0;
    }
}

//returns 0 if decode has success
void decode(const char *str, stats *res, int *i)
{
    int currentCharacter = 0, numberStartingIndex = *i;
    int index = *i;
    char buffer[20];
    for (index = index; str[index] != '-' && str[index] != '\0'; index++)
    {
        if (currentCharacter >= ASCII_CHARACTERS)
        {
            fprintf(stderr, "String decode error, too many entries.\n");
            return;
        }
        if (str[index] == '.' || str[index] == '\0')
        {
            memcpy(buffer, &str[numberStartingIndex], index - numberStartingIndex);
            buffer[index - numberStartingIndex] = '\0';
            res->frequencies[currentCharacter] += atoi(buffer);
            numberStartingIndex = index + 1;
            currentCharacter++;
        }
    }
    if (currentCharacter < ASCII_CHARACTERS - 1)
    {
        fprintf(stderr, "String decode error, too few entries. There were %d.\n", currentCharacter);
        return;
    }
    *i = index;
}

void decodeMultiple(const char *str, stats *array)
{
    char buffer[20];
    int statCounter = 0, i, idStartingIndex = 0;
    int stringLen = strlen(str);
    for (i = 0; i < stringLen; i++)
    {
        if (str[i] == '-')
        {
            idStartingIndex = i + 1;
        }
        if (str[i] == '_')
        {
            memcpy(buffer, &str[idStartingIndex], i - idStartingIndex);
            buffer[i - idStartingIndex] = '\0';
            array[statCounter].fileID = atoi(buffer);
            i++;
            decode(str, array + statCounter, &i);
            idStartingIndex = i + 1;
            statCounter++;
        }
    }
    return;
}

char *encode(const stats stat)
{
    char *buffer;
    allocWrapper(MAX_PIPE_CHARACTERS, sizeof(char), (void**)&buffer);
    int i, pointer;
    sprintf(buffer, "-%d_", stat.fileID);
    //printf("\nwrinting id: -%d_\n", stat.fileID);
    pointer = getDigits(stat.fileID) + 2;
    for (i = 0; i < ASCII_CHARACTERS; i++)
    {
        sprintf(buffer + pointer, "%d", stat.frequencies[i]);
        pointer += getDigits(stat.frequencies[i]);
        if (i < ASCII_CHARACTERS - 1)
        {
            sprintf(buffer + pointer, ".");
            pointer++;
        }
    }
    buffer[pointer + 1] = '\0';
    return buffer;
}

char *encodeMultiple(stats *statsArray, int len)
{
    char *res;
    int offset = 0, i;
    allocWrapper(MAX_PIPE_CHARACTERS * len, sizeof(char), (void **)&res);
    for (i = 0; i < len; i++)
    {
        char *str = encode(statsArray[i]);
        int stringLen = strlen(str);
        memcpy(res + offset, str, stringLen);
        removeFromGCAndFree(str);
        offset += stringLen;
    }
    return res;
}

void removeFileFromStatsArray(stats *s, int index, int size)
{
    if (size == 0)
        return;
    if (size > 1)
    {
        //strcpy(c->files[i], c->files[c->filesCount - 1]);
        s[index] = s[size - 1];
    }
}

//sums the content of first and second modifying first
void sumStats(stats *first, const stats *second)
{
    int i;
    for (i = 0; i < ASCII_CHARACTERS; i++)
    {
        first->frequencies[i] += second->frequencies[i];
    }
}

void printStats(const stats s)
{
    printf("stat:\n");
    int i;
    for (i = 0; i < ASCII_CHARACTERS; i++)
    {
        if (s.frequencies[i] != 0)
            printf("%c : %d\n", i, s.frequencies[i]);
    }
    printf("\n\n");
}
