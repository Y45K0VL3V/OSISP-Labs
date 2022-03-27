#include <sys/stat.h>
#include <time.h>
#include "stdio.h"
#include "dirent.h"
#include "fcntl.h"
#include "stdlib.h"
#include "string.h"

typedef struct List
{
    char* str;
    struct List* next;
} List;

void AddList(List ***head, char* str) {
    List* tmp = (List*)malloc(sizeof(List));
    tmp->str = calloc(strlen(str)+1, 1);
    strcat(tmp->str, str);
    tmp->next = (**head);
    (**head) = tmp;
}

typedef struct HistoryList
{
    int firstDesk;
    int secondDesk;
    struct HistoryList* next;
} HistoryList;

void AddHistory(HistoryList **head, int desk1, int desk2) {
    HistoryList *tmp = (HistoryList*)malloc(sizeof(HistoryList));
    tmp->firstDesk = desk1;
    tmp->secondDesk = desk2;
    tmp->next = (*head);
    (*head) = tmp;
}

int IsHistoryContain(HistoryList **head, int desk1, int desk2)
{
    HistoryList *curr = *head;
    while (curr != NULL)
    {
        if (curr->firstDesk == desk1 && curr->secondDesk == desk2)
            return 1;
        curr = curr->next;
    }

    return 0;
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

void GetAllFilePath(DIR** dir, List** paths, char* currPath);
int FileEquals(char* firstFilePath, char* secondFilePath);
void OutEqualFiles(char* firstPath, char* secondPath, FILE* outputStream);

HistoryList *history = NULL;

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        fprintf(stderr, "Invalid arguments amount!\n");
        fprintf(stderr, "First argument - 1st directory path.\n");
        fprintf(stderr, "Second argument - 2nd directory path.\n");
        fprintf(stderr, "Third argument - output file path.\n");
        return 0;
    }

    DIR *firstDir, *secondDir;
    if (!(SafeOpenDir(&firstDir, argv[1]) & SafeOpenDir(&secondDir, argv[2])))
        return 0;

    int fileDescriptor;
    if ((fileDescriptor = open(argv[3], O_WRONLY | O_TRUNC | O_CREAT, 0644)) == -1)
    {
        fprintf(stderr, "File error: Can't read file %s.\n", argv[3]);
        return 0;
    }

    FILE* outputFile;
    OpenFileDescriptor(&outputFile, fileDescriptor, "w");

    List *firstDirFiles = NULL, *secondDirFiles = NULL;
    GetAllFilePath(&firstDir, &firstDirFiles, argv[1]);
    GetAllFilePath(&secondDir, &secondDirFiles, argv[2]);

    List* currPathFirst = firstDirFiles;
    while (currPathFirst != NULL)
    {
        List* currPathSecond = secondDirFiles;
        while (currPathSecond != NULL)
        {
            if (FileEquals(currPathFirst->str, currPathSecond->str))
            {
                OutEqualFiles(currPathFirst->str, currPathSecond->str, stdout);
                OutEqualFiles(currPathFirst->str, currPathSecond->str, outputFile);
            }
            currPathSecond = currPathSecond->next;
        }
        currPathFirst = currPathFirst->next;
    }

    if (fclose(outputFile))
    {
        fprintf(stderr, "Error occurred while trying to close output file.\n");
        return 0;
    }

    return 1;
}
void GetAllFilePath(DIR** dir, List** paths, char* currPath)
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

        if (dirData->d_type != DT_DIR)
            AddList(&paths, newPath);
        else
        {
            if (!SafeOpenDir(&subDir,newPath))
            {
                free(newPath);
                continue;
            }
            GetAllFilePath(&subDir, paths, newPath);
        }

        free(newPath);
    }

    CloseDir(dir);
}

int FileEquals(char* firstFilePath, char* secondFilePath)
{
    int firstFileDescriptor, secondFileDescriptor;
    if (!(SafeGetFileDescriptor(&firstFileDescriptor, firstFilePath) &
          SafeGetFileDescriptor(&secondFileDescriptor, secondFilePath)))
        return 0;

    FILE *firstFile, *secondFile;
    if(!(OpenFileDescriptor(&firstFile, firstFileDescriptor, "r") & OpenFileDescriptor(&secondFile,secondFileDescriptor,"r")))
    {
        return 0;
    }

    char char1, char2;
    while((char1=fgetc(firstFile)) != EOF | (char2=fgetc(secondFile)) != EOF)
    {
        if (char1 != char2)
        {
            CloseFile(&firstFile);
            CloseFile(&secondFile);
            return 0;
        }
    }

    CloseFile(&firstFile);
    CloseFile(&secondFile);

    return 1;
}

void OutFileStat(char* path, struct stat* stat, FILE* stream)
{
    fprintf(stream, "\nFile\n");
    fprintf(stream, "Path: %s\n", path);
    fprintf(stream, "Size: %lldb\n", stat->st_size);
    fprintf(stream, "Create time: %s", ctime(&stat->st_ctim));
    fprintf(stream, "Mode: %lo\n",  (unsigned long)stat->st_mode);
    fprintf(stream, "Ind descriptor: %ld\n", stat->st_ino);
}

void OutEqualFiles(char* firstPath, char* secondPath, FILE* outputStream)
{
    struct stat* firstStat = (struct stat *)calloc(1, sizeof(struct stat));
    struct stat* secondStat = (struct stat *)calloc(1, sizeof(struct stat));
    stat(firstPath, firstStat);
    stat(secondPath, secondStat);

    if(firstStat->st_ino != secondStat->st_ino && !IsHistoryContain(&history,secondStat->st_ino, firstStat->st_ino))
    {
        AddHistory(&history, firstStat->st_ino, secondStat->st_ino);
        fprintf(outputStream, "------------------------\nThese files contain the same:\n");
        OutFileStat(firstPath, firstStat, outputStream);
        fprintf(outputStream, "                AND");
        OutFileStat(secondPath, secondStat, outputStream);
        fprintf(outputStream, "------------------------\n");
    }
    free(firstStat);
    free(secondStat);
}
