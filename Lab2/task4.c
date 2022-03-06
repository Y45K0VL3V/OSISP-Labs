#include "stdio.h"
#include "fcntl.h"
#include "string.h"
#include "ctype.h"
#include "stdlib.h"

int CloseFile(FILE* file)
{
    // fclose() returns 0, if success.
    if (fclose(file))
    {
        perror("Error occurred while trying to close file.");
        return 0;
    }

    return 1;
}

int IsNumber(char* input)
{
    for (int i = 0; i < strlen(input); i++)
    {
        if(!isdigit(input[i]))
            return 0;
    }

    return 1;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        perror("Invalid arguments amount!\nFirst argument - file path.\nSecond argument - amount of lines in group");
        return 0;
    }

    int descriptor;
    if ((descriptor = open(argv[1], O_RDONLY)) == -1)
    {
        perror("File error: Can't read file or it doesn't exist.");
        return 0;
    }

    if (!IsNumber(argv[2]))
    {
        perror("Cast error: Can't cast 2nd argument to unsigned int.");
        return 0;
    }

    int groupLineAmount = atoi(argv[2]);
    FILE* file = fdopen(descriptor, "r");

    char currChar;
    int currGroupLines = 0;
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
            currGroupLines = 0;
        }
    }

    if (!CloseFile(file))
        return 0;

    return 1;
}