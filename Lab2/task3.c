#include "stdio.h"
#include <fcntl.h>

// Exit, on press "Escape".
const char EXT_CHAR = 27;

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

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        perror("Invalid arguments amount!\nFirst argument - file path to write in.");
        return 0;
    }

    int descriptor;
    if ((descriptor = open(argv[1], O_WRONLY | O_TRUNC | O_CREAT, 0644)) == -1)
    {
        perror("Can't get the file with entered path.");
        return 0;
    }

    FILE* file = fdopen(descriptor, "w");
    char inputChar;
    while ((inputChar = getchar()) != EXT_CHAR)
    {
        if(putc(inputChar, file) == EOF)
        {
            perror("Error occurred while writing in file.\n Check your file.");
            if (!CloseFile(file))
                return 0;
        }
    }

    if (!CloseFile(file))
        return 0;

    return 1;
}