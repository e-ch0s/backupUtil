#include <Windows.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define u64 uint64_t

#define i8 int8_t
#define i16 int16_t
#define i32 int32_t
#define i64 int64_t

#define DATE_BUFFER_SIZE 20
#define OPTION_BUFFER_SIZE 10
#define DATA_ARRAY_SIZE 10000
#define MAX_PRINT_LENGTH 50
#define PATH_BUFFER_SIZE (MAX_PATH + 1)

#define CREATED_AFTER 1
#define CREATED_BEFORE -1
#define CREATED_ON_DATE 0

#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#define MIN(a,b) (((a) < (b)) ? (a) : (b))

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

typedef struct
{
    char sourceFileName[PATH_BUFFER_SIZE];
    char destinationFileName[PATH_BUFFER_SIZE];
} copyFileNames;

i8 validateDate(char *date, dateStruct *dateBuffer);
inline void removeTrailingByte(char *input, char *byte);
i8 validateFilterRange(dateStruct startDate, dateStruct endDate);
i8 fileTimeFilter(dateStruct filterDate, SYSTEMTIME creationDate);
i8 fileTimeCmp(SYSTEMTIME fileTime1, SYSTEMTIME fileTime2);
void quicksort(fileInfo *array, i64 low, i64 high);
i64 partition(fileInfo *array, i64 low, i64 high);
i8 mergeDirFileName(char *buffer, char *directory, char *fileName, u32 bufferSize);

int main(int argc, char **argv)
{
    HANDLE heapHandle = GetProcessHeap();
    if (heapHandle == NULL)
    {
        return -1;
    }
    char option[OPTION_BUFFER_SIZE] = {0};
    char workingDirectory[PATH_BUFFER_SIZE] = {0};

    do
    {
        printf("Welcome to backupUtil\n");
        GetCurrentDirectoryA(PATH_BUFFER_SIZE, workingDirectory);
        printf("Current working directory: %s\n", workingDirectory);
        printf("Select option:\n");
        printf("1: Copy to current working directory\n");
        printf("2: Copy from current working directory\n");
        printf("3: Custom\n");
        printf("Enter option: ");
        fgets(option, sizeof(option), stdin);
    } while (strcmp(option, "1\n") && strcmp(option, "2\n") && strcmp(option, "3\n"));

    u8 selectDir = 1;

    char sourceDirectory[PATH_BUFFER_SIZE] = {0};
    char destinationDirectory[PATH_BUFFER_SIZE] = {0};
    char sourceDirectoryFilePath[PATH_BUFFER_SIZE] = {0};
    char destinationDirectoryFilePath[PATH_BUFFER_SIZE] = {0};
    char selectDirOption[OPTION_BUFFER_SIZE] = {0};

    fileInfo *fileData = (fileInfo *) HeapAlloc(heapHandle, HEAP_ZERO_MEMORY, sizeof(fileInfo) * DATA_ARRAY_SIZE);
    if (fileData == NULL)
    {
        return -1;
    }
    HANDLE fileHandle = NULL;
    
    if (strcmp(option, "1\n") == 0)
    {
        snprintf(destinationDirectory, sizeof(destinationDirectory), "%s", workingDirectory);
        while (selectDir)
        {
            printf("Enter source directory:\n");
            fgets(sourceDirectory, sizeof(sourceDirectory), stdin);
            removeTrailingByte(sourceDirectory, "\n");
            printf("Is \"%s\" the source directory?\n", sourceDirectory);
            printf("Select Y/N:\n");
            fgets(selectDirOption, sizeof(selectDirOption), stdin);
            if (strcmp(selectDirOption, "Y\n") == 0 || strcmp(selectDirOption, "y\n") == 0) 
            {
                snprintf(sourceDirectoryFilePath, sizeof(sourceDirectoryFilePath), "%s\\%s", sourceDirectory, "*");
                if (mergeDirFileName(sourceDirectoryFilePath, sourceDirectory, "*", sizeof(sourceDirectoryFilePath)) == 0)
                {
                    fileHandle = FindFirstFileA(sourceDirectoryFilePath, &(fileData->data));
                    if (fileHandle != INVALID_HANDLE_VALUE)
                    {
                        selectDir = 0;
                    }
                    else 
                    {
                        printf("Invalid directory. Try again.\n");
                    }
                }
                else 
                {
                    printf("File path exceeds limit.\n");
                }
            }
        }
    }
    else if(strcmp(option, "2\n") == 0)
    {
        snprintf(sourceDirectory, sizeof(sourceDirectory), "%s", workingDirectory);
        while (selectDir)
        {
            printf("Enter destination directory:\n");
            fgets(destinationDirectory, sizeof(destinationDirectory), stdin);
            removeTrailingByte(destinationDirectory, "\n");
            printf("Is \"%s\" the destination directory?\n", destinationDirectory);
            printf("Select Y/N:\n");
            fgets(selectDirOption, sizeof(selectDirOption), stdin);
            if (strcmp(selectDirOption, "Y\n") == 0 || strcmp(selectDirOption, "y\n") == 0) 
            {
                if (mergeDirFileName(sourceDirectoryFilePath, sourceDirectory, "*", sizeof(sourceDirectoryFilePath)) == 0)
                {
                    WIN32_FIND_DATAA dummyData = {0};
                    fileHandle = FindFirstFileA(sourceDirectoryFilePath, &dummyData);
                    if (fileHandle != INVALID_HANDLE_VALUE)
                    {
                        selectDir = 0;
                        fileHandle = FindFirstFileA(sourceDirectoryFilePath, &(fileData->data));
                    }
                    else 
                    {
                        printf("Invalid directory. Try again.\n");
                    }
                }
                else 
                {
                    printf("File path exceeds limit.\n");
                }
            }
        }
    }
    else
    {
    }

    size_t fileIndex = 1;

    while (FindNextFileA(fileHandle, &(fileData + fileIndex++)->data) != 0 && fileIndex < DATA_ARRAY_SIZE)
        ;
    FindClose(fileHandle);

    if (GetLastError() == ERROR_NO_MORE_FILES)
    {
        printf("No more files\n");
        fileIndex--;
    }
    if (fileIndex == DATA_ARRAY_SIZE)
    {
        printf("Too many files\n");
        printf("Do you want to continue?\n");
    }
    for (u8 i = 0; i < fileIndex; ++i)
    {
        SYSTEMTIME time = {0};
        FileTimeToSystemTime(&fileData[i].data.ftCreationTime, &time);
        fileData[i].time = time;
        printf("File name: %10s Date created: %d/%d/%d\n", fileData[i].data.cFileName, time.wDay, time.wMonth, time.wYear);
    }
    printf("File count: %u\n", fileIndex);

    char startDate[DATE_BUFFER_SIZE] = {0};
    char endDate[DATE_BUFFER_SIZE] = {0};
    dateStruct start = {0};
    dateStruct end = {0};

    bool validRange = false;
    do
    {
        do
        {
            printf("Enter start date (DD/MM/YYYY):\n");
            fgets(startDate, sizeof(startDate), stdin);
            removeTrailingByte(startDate, "\n");
        } while(validateDate(startDate, &start) != 0);

        do
        {
            printf("Enter end date (DD/MM/YYYY):\n");
            fgets(endDate, sizeof(endDate), stdin);
            removeTrailingByte(endDate, "\n");
        } while (validateDate(endDate, &end) != 0);
        
        validRange = validateFilterRange(start, end);
        if (!validRange)
        {
            printf("Invalid range. Please try again.\n");
        }
    } while (!validRange);

    fileInfo *filteredFileData = (fileInfo *) HeapAlloc(heapHandle, HEAP_ZERO_MEMORY, sizeof(fileInfo) * fileIndex);
    if (filteredFileData == NULL)
    {
        return -1;
    }

    size_t filteredFileIndex = 0;
    for (u8 i = 0; i < fileIndex; ++i)
    {
        if (fileTimeFilter(start, fileData[i].time) >=CREATED_ON_DATE && fileTimeFilter(end, fileData[i].time) <= CREATED_ON_DATE && filteredFileIndex < fileIndex)
        {
            filteredFileData[filteredFileIndex] = fileData[i]; 
            ++filteredFileIndex;
        }
    }
    HeapFree(heapHandle, 0, fileData);

    quicksort(filteredFileData, 0, filteredFileIndex - 1);

    for(u8 i = 0; i < MIN(filteredFileIndex, MAX_PRINT_LENGTH); ++i)
    {
        printf("File name: %10s Date created: %d/%d/%d\n", filteredFileData[i].data.cFileName, filteredFileData[i].time.wDay, filteredFileData[i].time.wMonth, filteredFileData[i].time.wYear);
    }

    char *failedSourceFiles[50] = {0};
    char *failedDestinationFiles[50] = {0};
    u8 failedSourceCounter = 0;
    u8 failedDestinationCounter = 0;
    copyFileNames *successfulFiles =  (copyFileNames *) HeapAlloc(heapHandle, HEAP_ZERO_MEMORY, sizeof(copyFileNames) * filteredFileIndex);

    for (u8 i = 0; i < filteredFileIndex; ++i)
    {
        char sourceFileBuffer[PATH_BUFFER_SIZE] = {0};
        char destinationFileBuffer[PATH_BUFFER_SIZE] = {0};

        if (mergeDirFileName(sourceFileBuffer, sourceDirectory, filteredFileData[i].data.cFileName,sizeof(sourceFileBuffer)))
        {
            if (failedSourceCounter < 50)
            {
                failedSourceFiles[failedSourceCounter++] = filteredFileData[i].data.cFileName;
            }
            else
            {
                printf("Too many source files failed to be opened. Program exiting\n");
                return -1;
            }
        }
        else if (mergeDirFileName(destinationFileBuffer, destinationDirectory, filteredFileData[i].data.cFileName, sizeof(destinationFileBuffer)))
        {
            if (failedDestinationCounter < 50)
            {
                failedDestinationFiles[failedDestinationCounter++] = filteredFileData[i].data.cFileName;
            }
            else
            {
                printf("Too many destination files failed to be created. Program exiting\n");
                return -1;
            }
        }
        else
        {
        }
    }
}

inline void removeTrailingByte(char *input, char *byte)
{
    input[strcspn(input, byte)] = 0;
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

i8 validateFilterRange(dateStruct startDate, dateStruct endDate)
{
    if (startDate.year < endDate.year)
    {
        return true;
    }
    else if (startDate.month < endDate.month)
    {
        return true;
    }
    else if (startDate.day <= endDate.day)
    {
        return true;
    }
    return false;
}

i8 fileTimeFilter(dateStruct filterDate, SYSTEMTIME creationDate)
{
    if (filterDate.year < creationDate.wYear)
    {
        return CREATED_AFTER;
    }
    else if(filterDate.year > creationDate.wYear)
    {
        return CREATED_BEFORE;
    }
    else if (filterDate.month < creationDate.wMonth)
    {
        return CREATED_AFTER;
    }
    else if (filterDate.month > creationDate.wMonth)
    {
        return CREATED_BEFORE;
    }
    else if (filterDate.day < creationDate.wDay)
    {
        return CREATED_AFTER;
    }
    else if (filterDate.day > creationDate.wDay)
    {
        return CREATED_BEFORE;
    }
    return CREATED_ON_DATE;
}

i8 fileTimeCmp(SYSTEMTIME fileTime1, SYSTEMTIME fileTime2)
{
    if (fileTime1.wYear > fileTime2.wYear)
    {
        return CREATED_AFTER;
    }
    else if(fileTime1.wYear < fileTime2.wYear)
    {
        return CREATED_BEFORE;
    }
    else if (fileTime1.wMonth > fileTime2.wMonth)
    {
        return CREATED_AFTER;
    }
    else if (fileTime1.wMonth < fileTime2.wMonth)
    {
        return CREATED_BEFORE;
    }
    else if (fileTime1.wDay > fileTime2.wDay)
    {
        return CREATED_AFTER;
    }
    else if (fileTime1.wDay < fileTime2.wDay)
    {
        return CREATED_BEFORE;
    }
    return CREATED_ON_DATE;
}

void quicksort(fileInfo *array, i64 low, i64 high)
{
    if (low >= high)
    {
        return;
    }
    i64 pivot = partition(array, low, high);

    quicksort(array, low, pivot -1);
    quicksort(array, pivot + 1, high);

    return;
}

i64 partition(fileInfo *array, i64 low, i64 high)
{
    fileInfo pivot = array[high];

    i64 index = low - 1;

    for (i64 i = low; i < high; ++i)
    {
        if (fileTimeCmp(array[i].time, pivot.time) <= CREATED_ON_DATE)
        {
            index++;
            fileInfo temp = array[i];
            array[i] = array[index];
            array[index] = temp;
        }
    }
    index++;
    array[high] = array[index];
    array[index] = pivot;

    return index;
}

i8 mergeDirFileName(char *buffer, char *directory, char *fileName, u32 bufferSize)
{
    removeTrailingByte(directory, "*");

    // 1 for inserting /
    if (strlen(directory) + strlen(fileName) + 1 > MAX_PATH)
    {
        return -1;
    }
    
    snprintf(buffer, bufferSize , "%s\\%s", directory, fileName);

    return 0;
}
