#include <Windows.h>
#include <stdio.h>
#include <string.h>
#include <winerror.h>

#define OPTIONBUFFERSIZE 10
#define DATAARRAYSIZE 2750

void validatePath(char *path);

int main(int argc, char **argv)
{
    char option[OPTIONBUFFERSIZE] = {0};
    char workingDirectory[MAX_PATH] = {0};

    do
    {
        printf("Welcome to backupUtil\n");
        GetCurrentDirectoryA(MAX_PATH, workingDirectory);
        printf("Current working directory: %s\n", workingDirectory);
        printf("Select option:\n");
        printf("1: Copy to current working directory\n");
        printf("2: Copy from current working directory\n");
        printf("3: Custom\n");
        printf("Enter option: ");
        fgets(option, sizeof(option), stdin);
    } while (strcmp(option, "1\n") && strcmp(option, "2\n") && strcmp(option, "3\n"));

    int selectDir = 1;
    char secondaryDirectory[MAX_PATH] = {0};
    char selectDirOption[OPTIONBUFFERSIZE] = {0};

    if (strcmp(option, "1\n") == 0)
    {
        while (selectDir)
        {
            printf("Enter source directory:\n");
            fgets(secondaryDirectory, sizeof(secondaryDirectory), stdin);
            validatePath(secondaryDirectory);
            printf("Is \"%s\" the source directory?\n", secondaryDirectory);
            printf("Select Y/N:\n");
            fgets(selectDirOption, sizeof(selectDirOption), stdin);
            if (strcmp(selectDirOption, "Y\n") == 0 || strcmp(selectDirOption, "y\n") == 0) 
            {
                selectDir = 0;
            }
        }
        strcat_s(secondaryDirectory , MAX_PATH, "\\*");
        WIN32_FIND_DATA fileData[DATAARRAYSIZE] = {0};
        HANDLE fileHandle = FindFirstFileA(secondaryDirectory, fileData);
        if (fileHandle == INVALID_HANDLE_VALUE)
        {
            printf("Invalid directory\n");
        }
        int fileIndex = 1;
        while (FindNextFileA(fileHandle, fileData + fileIndex++) != 0 && fileIndex < DATAARRAYSIZE)
            ;
        for (int i = 0; i < fileIndex; ++i)
        {
            printf("File name: %s\n", fileData[i].cFileName);
        }
        if (GetLastError() == ERROR_NO_MORE_FILES)
        {
            printf("No more files\n");
        }
        if (fileIndex == DATAARRAYSIZE)
        {
            printf("Too many files\n");
        }
    }
    else if(strcmp(option, "2\n"))
    {
    }
    else
    {
    }
}

void validatePath(char *path)
{
    for (int i = 0; path[i] != '\0' && i < MAX_PATH; ++i)
    {
        if (path[i] == '\n')
        {
            path[i] = '\0';
        }
    }
}
