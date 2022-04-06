#include "stdio.h"
#include "dirent.h"
#include "fcntl.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include "limits.h"
#include "errno.h"
#include "wait.h"

typedef struct List
{
    char* str;
    struct List* next;
} List;

void AddList(List **head, char* str) {
    List* tmp = (List*)malloc(sizeof(List));
    tmp->str = calloc(strlen(str)+1, 1);
    strcat(tmp->str, str);
    tmp->next = (*head);
    (*head) = tmp;
}

int SafeOpenDir(DIR** dir, char* dirPath)
{
    if ((*dir = opendir(dirPath)) == NULL)
    {
        fprintf( stderr, "Can't open directory %s.\n", dirPath);
        return 0;
    }
    return 1;
}

int CloseDir(DIR** dir)
{
    if (closedir(*dir))
    {
        fprintf(stderr, "Error occurred while trying to close directory.\n");
        return 0;
    }

    return 1;
}

// For files to read.
int SafeGetFileDescriptor(int* fileDescriptor, char* filePath)
{
    if ((*fileDescriptor = open(filePath, O_RDONLY)) == -1)
    {
        fprintf(stderr, "File error: Can't read file %s.\n", filePath);
        return 0;
    }
    return 1;
}

int OpenFileDescriptor(FILE **destFile, int descriptor, char* mode)
{
    if ((*destFile = fdopen(descriptor, mode)) == NULL)
    {
        fprintf(stderr, "Can't get the file with entered path.\n");
        return 0;
    }

    return 1;
}

int CloseFile(FILE** file)
{
    // fclose() returns 0, if success.
    if (fclose(*file))
    {
        fprintf(stderr, "Error occurred while trying to close file.\n");
        return 0;
    }

    return 1;
}

void GetAllFilePath(DIR** dir, char* currPath);
void FileFindWord(char* filepath, char* word);

List* dirFiles = NULL;

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        fprintf(stderr, "Invalid arguments amount!\n");
        fprintf(stderr, "First argument - 1st directory path.\n");
        fprintf(stderr, "Second argument - wanted word.\n");
        fprintf(stderr, "Third argument - max processes number.\n");
        return -1;
    }

    DIR *firstDir;
    if (!SafeOpenDir(&firstDir, argv[1]))
        return -1;

    char *endPos;
    long maxProcessesAmount = strtol(argv[3], &endPos,10);

    if ((errno == ERANGE && (maxProcessesAmount == LONG_MAX || maxProcessesAmount == LONG_MIN)) || (errno != 0 && maxProcessesAmount == 0))
    {
        fprintf(stderr, "Error: lines amount out of range.\n");
        return -1;
    }
    else
        if (maxProcessesAmount < 1)
        {
            fprintf(stderr, "Error: number less than 1.\n");
            return -1;
        }
        else
            if (endPos[0] != '\0' || endPos == argv[3])
            {
                fprintf(stderr, "Error: can't cast entered text to int.\n");
                return -1;
            }

    GetAllFilePath(&firstDir, argv[1]);
    List* currPathFirst = dirFiles;
    int processesAmount = 0;
    while (currPathFirst != NULL)
    {
        if (processesAmount == maxProcessesAmount)
        {
            if (wait(NULL) == -1)
            {
                fprintf(stderr, "Error occurred, while waiting child process closing.\n");
            }
            processesAmount--;
        }

        pid_t pid_t = fork();
        switch (pid_t) {
            case 0:
                FileFindWord(currPathFirst->str, argv[2]);
                return 0;
            case -1:
                fprintf(stderr, "Error: can't create child process");
                break;
            default:
                processesAmount++;
                break;
        }
        currPathFirst = currPathFirst->next;
    }

    while (1)
    {
        if (wait(NULL) == -1)
        {
            if (errno == ECHILD)
                break;
            fprintf(stderr, "Error occurred, while waiting child process closing.\n");
        }
    }

    return 0;
}
void GetAllFilePath(DIR** dir, char* currPath)
{
    struct dirent *dirData;
    DIR* subDir;
    while ((dirData = readdir(*dir)) != NULL)
    {
        if (!strcmp(dirData->d_name, ".") || !strcmp(dirData->d_name, ".."))
            continue;

        char* newPath = calloc(strlen(currPath) + strlen(dirData->d_name)+2, 1);
        strcat(newPath, currPath);
        if (currPath[strlen(currPath) - 1] != '/')
            strcat(newPath, "/");
        strcat(newPath, dirData->d_name);

        if (dirData->d_type == DT_REG)
            AddList(&dirFiles, newPath);
        else
        {
            if (dirData->d_type != DT_DIR || !SafeOpenDir(&subDir,newPath))
            {
                free(newPath);
                continue;
            }
            GetAllFilePath(&subDir, newPath);
        }

        free(newPath);
    }

    CloseDir(dir);
}

void FileFindWord(char* filepath, char* word)
{
    int descriptor;
    if (!SafeGetFileDescriptor(&descriptor, filepath))
        return;

    FILE* file;
    if (!OpenFileDescriptor(&file, descriptor, "r"))
        return;

    int checkedBytesCount = 0;
    int matchedWordsAmount = 0;

    int wantedLength = strlen(word);
    int currLength = 0;
    char currChar;
    while((currChar = fgetc(file)) != EOF)
    {
        checkedBytesCount++;
        if (currChar == ' ' || currChar == '\n' || currChar == '\t' || currChar == '\r')
        {
            if (currLength == wantedLength)
                matchedWordsAmount++;

            currLength = 0;
        }
        else
        {
            if (currChar == word[currLength % wantedLength])
                currLength++;
        }
    }

    if (currLength == wantedLength)
        matchedWordsAmount++;

    printf("PID: %i Bytes: %10i Words: %10i Path: %s\n", getpid(), checkedBytesCount, matchedWordsAmount, filepath);
    CloseFile(&file);
}
