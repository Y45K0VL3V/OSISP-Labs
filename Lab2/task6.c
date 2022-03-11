#include "stdio.h"
#include "dirent.h"
#include "unistd.h"

void PrintDirData(char* dirPath)
{
    DIR* dir;
    if ((dir = opendir(dirPath)) == NULL)
    {
        fprintf( stderr, "Can't open directory %s.", dirPath);
        return;
    }

    struct dirent *dirData;
    while ((dirData = readdir(dir)) != NULL)
        printf("%s\n", dirData->d_name);

    if (closedir(dir) == -1)
        fprintf( stderr, "Can't close directory %s.", dirPath);
}

int main(int argc, char *argv[])
{
    char currDirPath[256];
    if (!getcwd(currDirPath, 256))
    {
        perror("Can't get current directory name.");
        return 0;
    }

    printf("Current directory:\n");
    PrintDirData(currDirPath);

    printf("\n");

    printf("Root directory:\n");
    PrintDirData("/");

    return 1;
}