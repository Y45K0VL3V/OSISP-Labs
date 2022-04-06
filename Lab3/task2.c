#include "stdio.h"
#include "stdlib.h"
#include "sys/types.h"
#include "unistd.h"
#include "time.h"
#include "sys/timeb.h"
#include "string.h"
#include "errno.h"
#include "wait.h"

#define TIME_STR_SIZE 12

void OutProcessInfo()
{
    struct timeb timebInfo;
    ftime(&timebInfo);
    struct tm *timeNow = localtime(&timebInfo.time);
    printf("PID: %i\nPPID: %i\n%02i:%02i:%02i:%03i\n\n", getpid(), getppid(), timeNow->tm_hour, timeNow->tm_min, timeNow->tm_sec, timebInfo.millitm);
}

int main()
{
    printf("Main process:\n\n");
    OutProcessInfo();

    printf("Children:\n\n");
    for(int i = 1; i <= 2; i++)
    {
        pid_t pid = fork();
        switch (pid)
        {
            case 0:
                OutProcessInfo();
                return 0;
            case -1:
                fprintf(stderr, "Error: can't create child process");
                break;
        }
    }

    system("ps -x");
    
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
