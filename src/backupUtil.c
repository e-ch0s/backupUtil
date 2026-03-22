#include <Windows.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define u64 uint64_t

#define i8 int8_t
#define i16 int16_t
#define i32 int32_t
#define i64 int64_t

#define DATEBUFFERSIZE 20
#define OPTIONBUFFERSIZE 10
#define DATAARRAYSIZE 2750

typedef struct
{
    u8 day;
    u8 month;
    u16 year;
} dateStruct;

typedef struct 
{
    WIN32_FIND_DATAA data;
    SYSTEMTIME time;
} fileInfo;

i8 validateDate(char *date, dateStruct *dateBuffer);
inline void removeLineFeed(char *input);

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

    u8 selectDir = 1;
    char secondaryDirectory[MAX_PATH] = {0};
    char selectDirOption[OPTIONBUFFERSIZE] = {0};

    if (strcmp(option, "1\n") == 0)
    {
        while (selectDir)
        {
            printf("Enter source directory:\n");
            fgets(secondaryDirectory, sizeof(secondaryDirectory), stdin);
            removeLineFeed(secondaryDirectory);
            printf("Is \"%s\" the source directory?\n", secondaryDirectory);
            printf("Select Y/N:\n");
            fgets(selectDirOption, sizeof(selectDirOption), stdin);
            if (strcmp(selectDirOption, "Y\n") == 0 || strcmp(selectDirOption, "y\n") == 0) 
            {
                selectDir = 0;
            }
        }
        strcat_s(secondaryDirectory , MAX_PATH, "\\*");

        fileInfo fileData[DATAARRAYSIZE] = {0};

        HANDLE fileHandle = FindFirstFileA(secondaryDirectory, &(fileData->data));
        if (fileHandle == INVALID_HANDLE_VALUE)
        {
            printf("Invalid directory\n");
        }

        u16 fileIndex = 1;
        while (FindNextFileA(fileHandle, &(fileData + fileIndex++)->data) != 0 && fileIndex < DATAARRAYSIZE)
            ;
        FindClose(fileHandle);
        if (GetLastError() == ERROR_NO_MORE_FILES)
        {
            printf("No more files\n");
            fileIndex--;
        }
        if (fileIndex == DATAARRAYSIZE)
        {
            printf("Too many files\n");
            printf("Do you want to continue?\n");
        }
        for (u8 i = 0; i < fileIndex; ++i)
        {
            SYSTEMTIME time = {0};
            FileTimeToSystemTime(&fileData[i].data.ftCreationTime, &time);
            printf("File name: %10s Date created: %d/%d/%d\n", fileData[i].data.cFileName, time.wDay, time.wMonth, time.wYear);
        }

        char startDate[DATEBUFFERSIZE] = {0};
        char endDate[DATEBUFFERSIZE] = {0};
        dateStruct start = {0};
        dateStruct end = {0};

        do
        {
            printf("Enter start date (DD/MM/YYYY):\n");
            fgets(startDate, sizeof(startDate), stdin);
            removeLineFeed(startDate);
        } while(validateDate(startDate, &start) != 0);

        do
        {
            printf("Enter end date (DD/MM/YYYY):\n");
            fgets(endDate, sizeof(endDate), stdin);
        } while(validateDate(endDate, &end) != 0);
    }
    else if(strcmp(option, "2\n"))
    {
    }
    else
    {
    }
}

inline void removeLineFeed(char *input)
{
    input[strcspn(input, "\n")] = 0;
}

i8 validateDate(char *date, dateStruct *dateBuffer)
{
    u8 daysOfMonth[][12] = {
        {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
        {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
    };

    u8 leap = 0;
    char day[3] = {0};
    char month[3] = {0};
    char year[5] = {0};

    if (date[2] != '/' || date[5] != '/' || strlen(date) != 10)
    {
        return -1;
    }

    for (u8 i = 0; i < strlen(date); ++i)
    {
        if (i != 2 && i != 5)
        {
            if (isdigit(date != 0))
            {
                return -1;
            }
        }
    }

    snprintf(day, sizeof(day), "%s", date);
    snprintf(month, sizeof(month), "%s", &date[3]);
    snprintf(year, sizeof(year), "%s", &date[6]);
    
    u8 dayValue = (u8) strtol(day, NULL, 0);
    u8 monthValue = (u8) strtol(month, NULL, 0);
    u16 yearValue = (u16) strtol(year, NULL, 0);
    printf("%d/%d/%d\n", dayValue, monthValue, yearValue);

    if ((yearValue % 4 == 0 && yearValue % 100 != 0) || (yearValue % 400 == 0))
    {
        leap = 1;
    }

    if (monthValue > 12 || monthValue < 1|| dayValue > daysOfMonth[leap][monthValue - 1] || dayValue < 1 || yearValue < 1)
    {
        return -1;
    }

    dateBuffer->day = dayValue;
    dateBuffer->month = monthValue;
    dateBuffer->year = yearValue;

    return 0;
}
