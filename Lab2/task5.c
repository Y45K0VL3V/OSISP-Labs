#include "stdio.h"
#include "fcntl.h"
#include "sys/types.h"
#include "sys/stat.h"

int CloseFile(FILE* file, char* errorText)
{
    // fclose() returns 0, if success.
    if (fclose(file))
    {
        fprintf(stderr, errorText);
        return 0;
    }

    return 1;
}

int OpenFileDescriptor(FILE **destFile, int descriptor, char* mode){
    if ((*destFile = fdopen(descriptor, mode)) == NULL)
    {
        fprintf(stderr, "Can't get the file with entered path.\n");
        return 0;
    }

    return 1;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Invalid arguments amount!\nFirst argument - source file path.\nSecond argument - dest file path\n");
        return 0;
    }

    int srcDescriptor = open(argv[1], O_RDONLY);
    if (srcDescriptor == -1)
    {
        fprintf(stderr, "File error: Can't read source file or it doesn't exist.\n");
        return 0;
    }

    int destDescriptor = open(argv[2], O_WRONLY | O_TRUNC | O_CREAT, 0644);
    if (destDescriptor == -1)
    {
        fprintf(stderr, "File error: Can't open destination file.\n");
        return 0;
    }

    struct stat srcFileStat;
    fstat(srcDescriptor, &srcFileStat);
    chmod(argv[2], srcFileStat.st_mode);

    FILE *srcFile, *destFile;
    if(!(OpenFileDescriptor(&srcFile, srcDescriptor, "r") & OpenFileDescriptor(&destFile,destDescriptor,"w")))
    {
        return 0;
    }

    char currChar;
    while ((currChar = getc(srcFile)) != EOF)
        putc(currChar, destFile);

    if (!(CloseFile(srcFile, "Error occurred while trying to close source file.") &
        CloseFile(destFile, "Error occurred while trying to close destination file.")))
        return 0;

    return 1;
}
