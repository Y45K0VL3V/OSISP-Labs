#include "stdio.h"
#include "fcntl.h"
#include "string.h"
#include "ctype.h"
#include "stdlib.h"
#include "limits.h"
#include "errno.h"

int CloseFile(FILE* file)
{
    // fclose() returns 0, if success.
    if (fclose(file))
    {
        fprintf(stderr, "Error occurred while trying to close file.\n");
        return 0;
    }

    return 1;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Invalid arguments amount!\nFirst argument - file path.\nSecond argument - amount of lines in group\n");
        return 0;
    }

    int descriptor;
    if ((descriptor = open(argv[1], O_RDONLY)) == -1)
    {
        fprintf(stderr, "File error: Can't read file or it doesn't exist.\n");
        return 0;
    }

    char *endPos;
    long groupLineAmount = strtol(argv[2], &endPos,10);

    if ((errno == ERANGE && (groupLineAmount == LONG_MAX || groupLineAmount == LONG_MIN)) || (errno != 0 && groupLineAmount == 0))
    {
        fprintf(stderr, "Error: lines amount out of range.\n");
        return 0;
    }
    else
        if (groupLineAmount < 0)
        {
            fprintf(stderr, "Error: number less than 0.\n");
            return 0;
        }
        else
            if (endPos[0] != '\0' || endPos == argv[2])
            {
                fprintf(stderr, "Error: can't cast entered text to int.\n");
                return 0;
            }

    FILE* file = fdopen(descriptor, "r");
    if (file == NULL)
    {
        fprintf(stderr, "Can't get the file with entered path.\n");
        return 0;
    }

    char currChar;
    long currGroupLines = 0;
    while((currChar = fgetc(file)) != EOF)
    {
        if (!currGroupLines || currGroupLines != groupLineAmount)
        {
            if (currChar == '\n')
            {
                putc('\n', stdout);
                currGroupLines++;
            }
            else
                putc(currChar, stdout);
        }
        else
        {
            getc(stdin);
            putc(currChar, stdout);
            currGroupLines = 0;
        }
    }

    if (!CloseFile(file))
        return 0;

    return 1;
}