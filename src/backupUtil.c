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

#define DATE_BUFFER_SIZE 12
#define OPTION_BUFFER_SIZE 3
#define DATA_ARRAY_SIZE 10000
#define MAX_PRINT_LENGTH 50
#define QUEUE_SIZE 500
#define MESSAGE_BUFFER_SIZE 500
#define LOG_PRINT_BUFFER_SIZE 359
#define TIME_BUFFER_SIZE 6
#define DAY_DATE_BUFFER_SIZE 3
#define MONTH_DATE_BUFFER_SIZE 3
#define YEAR_DATE_BUFFER_SIZE 5
#define HOUR_BUFFER_SIZE 3
#define MINUTE_BUFFER_SIZE 3

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
    u8 hour;
    u8 minutes;
} timeStruct;

typedef struct 
{
    WIN32_FIND_DATAA data;
    SYSTEMTIME time;
} fileInfo;

typedef struct
{
    char sourceFilePath[MAX_PATH];
    char destinationFilePath[MAX_PATH];
} copyFileNames;

typedef struct
{
    char *message;
    char *directory;
    char *fileName;
    SYSTEMTIME time;
} logInfo;

i8 clearStdin(char *buffer, size_t bufferSize);
i8 validateDate(char *date, dateStruct *dateBuffer);
inline i8 removeTrailingByte(char *input, char *byte);
i8 validateFilterRange(dateStruct startDate, dateStruct endDate);
i8 fileTimeFilter(dateStruct filterDate, SYSTEMTIME creationDate);
i8 fileTimeCmp(SYSTEMTIME fileTime1, SYSTEMTIME fileTime2);
void quicksort(fileInfo *array, i64 low, i64 high);
i64 partition(fileInfo *array, i64 low, i64 high);
i8 mergeDirFileName(char *buffer, char *directory, char *fileName, u32 bufferSize);
i8 GetDirFromFilePath(char *buffer, char *filePath, size_t bufferSize);
void logger(HANDLE logFileHandle, HANDLE heapHandle, logInfo *info);
void printLogLine(HANDLE logFileHandle);
logInfo createLogInfo(char *message, char *directory, char *fileName);
i8 GetFileFromFilePath(char *buffer, char *filePath, char *directory, size_t bufferSize);

int main(int argc, char **argv)
{
    char executableFilePath[MAX_PATH] = {0};
    char executableFileDirectory[MAX_PATH] = {0};
    char logFilePath[MAX_PATH] = {0};

    if (GetModuleFileNameA(NULL, executableFilePath, sizeof(executableFilePath)) == 0)
    {
        printf("Error loading executable. Exiting program.\n");
        return -1;
    }
    else if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
    {
        printf("Error loading executable. Insufficient buffer. Exiting program.\n");
        return -1;
    }

    GetDirFromFilePath(executableFileDirectory, executableFilePath, sizeof(executableFileDirectory));
    if (mergeDirFileName(logFilePath, executableFileDirectory, "log.txt", sizeof(logFilePath)) != 0)
    {
        printf("Error opening logger. Exiting program.\n");
        return -1;
    }

    HANDLE logHandle = CreateFileA(logFilePath, FILE_APPEND_DATA, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL); 
    if (logHandle == INVALID_HANDLE_VALUE)
    {
        printf("Error opening log file. Exiting program.\n");
        return -1;
    }
    if (GetLastError() == 0)
    {
        char logTitle[LOG_PRINT_BUFFER_SIZE] = {0};
        snprintf(logTitle, sizeof(logTitle), "|%-40s|%-10s|%-5s|%-40s|%-256s|\n", "ERROR DESCRIPTION", "DATE", "TIME", "FILE NAME", "DIRECTORY");
        DWORD logBytesWritten = 0;
        WriteFile(logHandle, logTitle, strlen(logTitle), &logBytesWritten, NULL);
        printLogLine(logHandle);
    }

    HANDLE heapHandle = GetProcessHeap();

    if (heapHandle == NULL)
    {
        printf("Failed to get process heap.\n");
        return -1;
    }
    char option[OPTION_BUFFER_SIZE] = {0};
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
        if (clearStdin(option, sizeof(option)))
        {
            logInfo info = createLogInfo("Invalid input buffer size. Exiting program.", "-", "-");
            logger(logHandle, heapHandle, &info);
            printf("Invalid input buffer size. Exiting program");
            return -1;
        }
    } while (strcmp(option, "1\n") && strcmp(option, "2\n") && strcmp(option, "3\n"));

    u8 selectDir = 1;

    char sourceDirectory[MAX_PATH] = {0};
    char destinationDirectory[MAX_PATH] = {0};
    char sourceDirectoryFilePath[MAX_PATH] = {0};
    char selectDirOption[OPTION_BUFFER_SIZE] = {0};

    fileInfo *fileData = (fileInfo *) HeapAlloc(heapHandle, HEAP_ZERO_MEMORY, sizeof(fileInfo) * DATA_ARRAY_SIZE);
    if (fileData == NULL)
    {
        logInfo info = createLogInfo("Failed to allocate file data memory arena.", "-", "-");
        logger(logHandle, heapHandle, &info); 
        return -1;
    }
    HANDLE sourceFileHandle = NULL;
    HANDLE destinationFileHandle = NULL;
    WIN32_FIND_DATAA dummyData = {0};
    
    if (strcmp(option, "1\n") == 0)
    {
        snprintf(destinationDirectory, sizeof(destinationDirectory), "%s", workingDirectory);
        while (selectDir)
        {
            printf("Enter source directory:\n");
            fgets(sourceDirectory, sizeof(sourceDirectory), stdin);
            if (clearStdin(sourceDirectory, sizeof(sourceDirectory)))
            {
                logInfo info = createLogInfo("Invalid input buffer size. Exiting program.", "-", "-");
                logger(logHandle, heapHandle, &info);
                printf("Invalid input buffer size. Exiting program");
                return -1;
            }
            removeTrailingByte(sourceDirectory, "\n");
            printf("Is \"%s\" the source directory?\n", sourceDirectory);
            printf("Select Y/N:\n");
            fgets(selectDirOption, sizeof(selectDirOption), stdin);
            if (clearStdin(selectDirOption, sizeof(selectDirOption)))
            {
                logInfo info = createLogInfo("Invalid input buffer size. Exiting program.", "-", "-");
                logger(logHandle, heapHandle, &info);
                printf("Invalid input buffer size. Exiting program");
                return -1;
            }
            if (strcmp(selectDirOption, "Y\n") == 0 || strcmp(selectDirOption, "y\n") == 0) 
            {
                if (strcmp(destinationDirectory, sourceDirectory) == 0)
                {
                    printf("Source directory is the same as destination directory. Try again.\n");
                }
                else if (mergeDirFileName(sourceDirectoryFilePath, sourceDirectory, "*", sizeof(sourceDirectoryFilePath)) == 0)
                {
                    sourceFileHandle = FindFirstFileA(sourceDirectoryFilePath, &(fileData->data));
                    destinationFileHandle = FindFirstFileA(destinationDirectory, &dummyData);
                    if (sourceFileHandle != INVALID_HANDLE_VALUE && destinationFileHandle != INVALID_HANDLE_VALUE)
                    {
                        selectDir = 0;
                    }
                    else if (sourceFileHandle == NULL)
                    {
                        printf("Invalid source directory. Try again.\n");
                    }
                    else if (destinationFileHandle == NULL)
                    {
                        logInfo info = createLogInfo("System provided invalid destination directory. Exited program.", destinationDirectory, "-");
                        logger(logHandle, heapHandle, &info);
                        printf("Invalid destination directory. System error. Exiting program.\n");
                        return -1;
                    }
                }
                else 
                {
                    printf("File path exceeds limit.\n");
                }
            }
            else if (strcmp(selectDirOption, "N\n") == 0 || strcmp(selectDirOption, "n\n") == 0)
            {
                return -1;
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
            if (clearStdin(destinationDirectory, sizeof(destinationDirectory)))
            {
                logInfo info = createLogInfo("Invalid input buffer size. Exiting program.", "-", "-");
                logger(logHandle, heapHandle, &info);
                printf("Invalid input buffer size. Exiting program");
                return -1;
            }
            removeTrailingByte(destinationDirectory, "\n");
            printf("Is \"%s\" the destination directory?\n", destinationDirectory);
            printf("Select Y/N:\n");
            fgets(selectDirOption, sizeof(selectDirOption), stdin);
            if (clearStdin(selectDirOption, sizeof(selectDirOption)))
            {
                logInfo info = createLogInfo("Invalid input buffer size. Exiting program.", "-", "-");
                logger(logHandle, heapHandle, &info);
                printf("Invalid input buffer size. Exiting program");
                return -1;
            }
            if (strcmp(selectDirOption, "Y\n") == 0 || strcmp(selectDirOption, "y\n") == 0) 
            {
                if (strcmp(destinationDirectory, sourceDirectory) == 0)
                {
                    printf("Source directory is the same as destination directory. Try again.\n");
                }
                else if (mergeDirFileName(sourceDirectoryFilePath, sourceDirectory, "*", sizeof(sourceDirectoryFilePath)) == 0)
                {
                    destinationFileHandle = FindFirstFileA(destinationDirectory, &dummyData);
                    sourceFileHandle = FindFirstFileA(sourceDirectoryFilePath, &(fileData->data));

                    if (sourceFileHandle != INVALID_HANDLE_VALUE && destinationFileHandle != INVALID_HANDLE_VALUE)
                    {
                        selectDir = 0;
                    }
                    else if (sourceFileHandle == NULL)
                    {
                        logInfo info = createLogInfo("System provided invalid source directory. Exiting program.", sourceDirectory, "-");
                        logger(logHandle, heapHandle, &info);
                        printf("Invalid source directory. System error. Exiting program.\n");
                        return -1;
                    }
                    else if (destinationFileHandle == NULL)
                    {
                        printf("Invalid destination directory. Try again.\n");
                    }
                }
                else 
                {
                    printf("File path exceeds limit.\n");
                }
            }
            else if (strcmp(selectDirOption, "N\n") == 0 || strcmp(selectDirOption, "n\n") == 0)
            {
                return -1;
            }
        }
    }
    else
    {
        while (selectDir)
        {
            printf("Enter source directory:\n");
            fgets(sourceDirectory, sizeof(sourceDirectory), stdin);
            if (clearStdin(sourceDirectory, sizeof(sourceDirectory)))
            {
                logInfo info = createLogInfo("Invalid input buffer size. Exiting program.", "-", "-");
                logger(logHandle, heapHandle, &info);
                printf("Invalid input buffer size. Exiting program");
                return -1;
            }
            removeTrailingByte(sourceDirectory, "\n");
            printf("Enter destination directory:\n");
            fgets(destinationDirectory, sizeof(destinationDirectory), stdin);
            if (clearStdin(destinationDirectory, sizeof(destinationDirectory)))
            {
                logInfo info = createLogInfo("Invalid input buffer size. Exiting program.", "-", "-");
                logger(logHandle, heapHandle, &info);
                printf("Invalid input buffer size. Exiting program");
                return -1;
            }
            removeTrailingByte(destinationDirectory, "\n");
            printf("Is \"%s\" the source directory and \"%s\" the destination directory?\n", sourceDirectory, destinationDirectory);
            printf("Select Y/N:\n");
            fgets(selectDirOption, sizeof(selectDirOption), stdin);
            if (clearStdin(selectDirOption, sizeof(selectDirOption)))
            {
                logInfo info = createLogInfo("Invalid input buffer size. Exiting program.", "-", "-");
                logger(logHandle, heapHandle, &info);
                printf("Invalid input buffer size. Exiting program");
                return -1;
            }
            if (strcmp(selectDirOption, "Y\n") == 0 || strcmp(selectDirOption, "y\n") == 0) 
            {
                if (strcmp(destinationDirectory, sourceDirectory) == 0)
                {
                    printf("Source directory is the same as destination directory. Try again.\n");
                }
                else if (mergeDirFileName(sourceDirectoryFilePath, sourceDirectory, "*", sizeof(sourceDirectoryFilePath)) == 0)
                {
                    destinationFileHandle = FindFirstFileA(destinationDirectory, &dummyData);
                    sourceFileHandle = FindFirstFileA(sourceDirectoryFilePath, &(fileData->data));
                    if (sourceFileHandle != INVALID_HANDLE_VALUE && destinationFileHandle != INVALID_HANDLE_VALUE)
                    {
                        selectDir = 0;
                    }
                    else if (sourceFileHandle == INVALID_HANDLE_VALUE)
                    {
                        printf("Invalid source directory. Try again.\n");
                    }
                    else if (destinationFileHandle == INVALID_HANDLE_VALUE)
                    {
                        printf("Invalid destination directory. Try again.\n");
                    }
                }
                else 
                {
                    printf("File path exceeds limit.\n");
                }
            }
            else if (strcmp(selectDirOption, "N\n") == 0 || strcmp(selectDirOption, "n\n") == 0)
            {
                return -1;
            }
        }
    }

    size_t fileIndex = 1;

    while (FindNextFileA(sourceFileHandle, &(fileData + fileIndex++)->data) != 0 && fileIndex < DATA_ARRAY_SIZE)
        ;
    FindClose(sourceFileHandle);

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
    for (u16 i = 0; i < fileIndex; ++i)
    {
        SYSTEMTIME time = {0};
        FileTimeToSystemTime(&fileData[i].data.ftCreationTime, &time);
        fileData[i].time = time;
        printf("File name: %10s Date created: %d/%d/%d\n", fileData[i].data.cFileName, time.wDay, time.wMonth, time.wYear);
    }
    printf("File count: %llu\n", fileIndex);

    char startDate[DATE_BUFFER_SIZE] = {0};
    char endDate[DATE_BUFFER_SIZE] = {0};
    dateStruct start = {0};
    dateStruct end = {0};

    i8 validRange = 0;
    do
    {
        do
        {
            printf("Enter start date (DD/MM/YYYY):\n");
            fgets(startDate, sizeof(startDate), stdin);
            if (clearStdin(startDate, sizeof(startDate)))
            {
                logInfo info = createLogInfo("Invalid input buffer size. Exiting program.", "-", "-");
                logger(logHandle, heapHandle, &info);
                printf("Invalid input buffer size. Exiting program");
                return -1;
            }
            removeTrailingByte(startDate, "\n");
        } while(validateDate(startDate, &start) != 0);

        do
        {
            printf("Enter end date (DD/MM/YYYY):\n");
            fgets(endDate, sizeof(endDate), stdin);
            if (clearStdin(endDate, sizeof(endDate)))
            {
                logInfo info = createLogInfo("Invalid input buffer size. Exiting program.", "-", "-");
                logger(logHandle, heapHandle, &info);
                printf("Invalid input buffer size. Exiting program");
                return -1;
            }
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
        logInfo info = createLogInfo("Failed to allocate memory for filtered file data. Exiting program", "-", "-");
        logger(logHandle, heapHandle, &info);
        return -1;
    }

    size_t filteredFileIndex = 0;
    for (u16 i = 0; i < fileIndex; ++i)
    {
        if (fileTimeFilter(start, fileData[i].time) >= CREATED_ON_DATE && fileTimeFilter(end, fileData[i].time) <= CREATED_ON_DATE 
                && filteredFileIndex < fileIndex && strcmp(fileData[i].data.cFileName, ".") && strcmp(fileData[i].data.cFileName, ".."))
        {
            int test = strcmp(fileData[i].data.cFileName, ".");
            filteredFileData[filteredFileIndex] = fileData[i]; 
            ++filteredFileIndex;
        }
    }
    HeapFree(heapHandle, 0, fileData);

    quicksort(filteredFileData, 0, filteredFileIndex - 1);

    for(u16 i = 0; i < MIN(filteredFileIndex, MAX_PRINT_LENGTH); ++i)
    {
        printf("File name: %10s Date created: %d/%d/%d\n", filteredFileData[i].data.cFileName, filteredFileData[i].time.wDay, 
                filteredFileData[i].time.wMonth, filteredFileData[i].time.wYear);
    }

    char *failedSourceFiles[50] = {0};
    char *failedDestinationFiles[50] = {0};
    u8 failedSourceCounter = 0;
    u8 failedDestinationCounter = 0;
    u8 totalFailedCounter = 0;
    copyFileNames *successfulFiles =  (copyFileNames *) HeapAlloc(heapHandle, HEAP_ZERO_MEMORY, sizeof(copyFileNames) * filteredFileIndex);
    size_t successfulFileCounter = 0;

    for (u16 i = 0; i < filteredFileIndex; ++i)
    {
        char sourceFileBuffer[MAX_PATH] = {0};
        char destinationFileBuffer[MAX_PATH] = {0};

        if (mergeDirFileName(sourceFileBuffer, sourceDirectory, filteredFileData[i].data.cFileName,sizeof(sourceFileBuffer)))
        {
            if (failedSourceCounter < 50)
            {
                failedSourceFiles[failedSourceCounter++] = filteredFileData[i].data.cFileName;
                totalFailedCounter++;
                char errorMessageBuffer[MESSAGE_BUFFER_SIZE] = {0};
                snprintf(errorMessageBuffer, sizeof(errorMessageBuffer), "%s %s", filteredFileData[i].data.cFileName, "could not be accessed.");
                logInfo info = createLogInfo(errorMessageBuffer, sourceDirectory, filteredFileData[i].data.cFileName);
                logger(logHandle, heapHandle, &info);
            }
            else
            {
                printf("Too many source files failed to be accesssed. Program exiting\n");
                logInfo info = createLogInfo("Too many source files failed to be accessed. Exiting program.", sourceDirectory, "-");
                logger(logHandle, heapHandle, &info);
                return -1;
            }
        }
        else if (mergeDirFileName(destinationFileBuffer, destinationDirectory, filteredFileData[i].data.cFileName, sizeof(destinationFileBuffer)))
        {
            if (failedDestinationCounter < 50)
            {
                failedDestinationFiles[failedDestinationCounter++] = filteredFileData[i].data.cFileName;
                totalFailedCounter++;
                char errorMessageBuffer[MESSAGE_BUFFER_SIZE] = {0};
                snprintf(errorMessageBuffer, sizeof(errorMessageBuffer), "%s %s", filteredFileData[i].data.cFileName, "could not be created.");
                logInfo info = createLogInfo(errorMessageBuffer, destinationDirectory, filteredFileData[i].data.cFileName);
                logger(logHandle, heapHandle, &info);
            }
            else
            {
                printf("Too many destination files failed to be created. Program exiting\n");
                logInfo info = createLogInfo("Too many destination files failed to be accessed. Exiting program.", destinationDirectory, filteredFileData[i].data.cFileName);
                logger(logHandle, heapHandle, &info);
                return -1;
            }
        }
        else
        {
            snprintf(successfulFiles[successfulFileCounter].sourceFilePath, sizeof(successfulFiles[successfulFileCounter].sourceFilePath), "%s", sourceFileBuffer);
            snprintf(successfulFiles[successfulFileCounter].destinationFilePath, sizeof(successfulFiles[successfulFileCounter].destinationFilePath), 
                    "%s", destinationFileBuffer);
            successfulFileCounter++;
        }
    }
    printf("%d files have failed.\n", totalFailedCounter);
    printf("Total files found: %llu\n", successfulFileCounter);

    char proceedOption[OPTION_BUFFER_SIZE] = {0};

    do 
    {
    printf("Do you want to proceed?");
    printf("Select Y/N:\n");
    fgets(proceedOption, sizeof(proceedOption), stdin);
    if (clearStdin(proceedOption, sizeof(proceedOption)))
    {
        logInfo info = createLogInfo("Invalid input buffer size. Exiting program.", "-", "-");
        logger(logHandle, heapHandle, &info);
        printf("Invalid input buffer size. Exiting program");
        return -1;
    }
    } while (strcmp(proceedOption, "Y\n") && strcmp(proceedOption, "y\n") && strcmp(proceedOption, "N\n") && strcmp(proceedOption, "n\n"));

    if (strcmp(proceedOption, "N\n") == 0 || strcmp(proceedOption, "n\n") == 0)
    {
        return -1;
    }

    size_t copiedFileCounter = 0;
    
    for (u16 i = 0; i < successfulFileCounter; ++i)
    {
        if (!CopyFile(successfulFiles[i].sourceFilePath, successfulFiles[i].destinationFilePath, TRUE))
        {
            char errorMessageBuffer[MESSAGE_BUFFER_SIZE] = {0};
            if (GetLastError() == ERROR_FILE_EXISTS)
            {
                printf("File exists. ");
                snprintf(errorMessageBuffer, sizeof(errorMessageBuffer), "%s %s", filteredFileData[i].data.cFileName, "could not be copied. It already exists.");
            }
            else 
            {
                snprintf(errorMessageBuffer, sizeof(errorMessageBuffer), "%s %s", filteredFileData[i].data.cFileName, "could not be copied.");
            }
            char sourceFileName[MAX_PATH] = {0};
            GetFileFromFilePath(sourceFileName, successfulFiles[i].sourceFilePath, sourceDirectory, sizeof(sourceFileName));
            logInfo info = createLogInfo(errorMessageBuffer, sourceDirectory, sourceFileName);
            logger(logHandle, heapHandle, &info);
            printf("Error copying %s.\n", (successfulFiles + i)->sourceFilePath);
        }
        else 
        {
            copiedFileCounter++;
        }
    }
    printf("Successfully copied %llu/%llu files.\n", copiedFileCounter, successfulFileCounter);
}

inline i8 removeTrailingByte(char *input, char *byte)
{
    if (strlen(input) <=0)
    {
        return -1;
    }

    size_t index = 0;
    if ((index = strcspn(input, byte)) == (strlen(input) - 1))
    {
        input[strcspn(input, byte)] = 0;
        return 0;
    }

    return -1;
}

i8 validateDate(char *date, dateStruct *dateBuffer)
{
    u8 daysOfMonth[][12] = {
        {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
        {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
    };

    u8 leap = 0;
    char day[DAY_DATE_BUFFER_SIZE] = {0};
    char month[MONTH_DATE_BUFFER_SIZE] = {0};
    char year[YEAR_DATE_BUFFER_SIZE] = {0};

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
        return 1;
    }
    else if (startDate.month < endDate.month)
    {
        return 1;
    }
    else if (startDate.day <= endDate.day)
    {
        return 1;
    }
    return 0;
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
    // 1 for inserting /
    if (strlen(directory) + strlen(fileName) + 1 > MAX_PATH)
    {
        return -1;
    }

    if (strcmp(directory, "") == 0 || strcmp(fileName, "") == 0)
    {
        return -1;
    }
    
    snprintf(buffer, bufferSize , "%s\\%s", directory, fileName);

    return 0;
}

i8 GetDirFromFilePath(char *buffer, char *filePath, size_t bufferSize)
{
    if (strlen(filePath) < 3)
    {
        return -1;
    }
    size_t index = strlen(filePath) - 1;
    if (index < 1)
    {
        return -1;
    }
    while (filePath[index--] != '\\' && index >= 3) 
        ;
    if (index == 1 && bufferSize >= (index + 3))
    {
        // + 3 only if the directory is the root drive
        snprintf(buffer, index + 3, "%s", filePath);
    }
    else if (bufferSize >= (index + 2))
    {
        // + 2 beacuse converting from length to index and including the null terminator
        snprintf(buffer, index + 2, "%s", filePath);
    }
    else 
    {
        return -1;
    }
    return 0;
}

i8 GetFileFromFilePath(char *buffer, char *filePath, char *directory, size_t bufferSize)
{
    u16 dirLength = strlen(directory);
    if (dirLength > MAX_PATH)
    {
        return -1;
    }
    if (strlen(filePath) > MAX_PATH - 1)
    {
        return -1;
    }
    snprintf(buffer, bufferSize, "%s", (filePath + dirLength + 1));
    return 0;
}

// tokenize the strings so i can print them on multiple lines
void logger(HANDLE logFileHandle, HANDLE heapHandle, logInfo *info)
{
    char *stringQueue[QUEUE_SIZE] = {0};
    char *tokens = HeapAlloc(heapHandle, HEAP_ZERO_MEMORY, sizeof(char) * 10000); 
    u16 numTokens = 0;
    u16 tokenArenaIndex = 0;
    u16 startOfTokenIndex = 0;

    for (u16 i = 0; i <= strlen(info->message); ++i)
    {
        if (info->message[i] == ' ' || info->message[i] == '\0')
        {
            snprintf(&tokens[tokenArenaIndex], (i - startOfTokenIndex + 1), "%s", &info->message[startOfTokenIndex]);
            stringQueue[numTokens] = &tokens[tokenArenaIndex];
            tokenArenaIndex += (i - startOfTokenIndex + 2);
            startOfTokenIndex = ++i;
            ++numTokens;
        }
    }

    char *tokenPointer = *stringQueue;
    u16 numTokensPrinted = 0;
    u8 firstLine = 1;
    u8 fileNameIndex = 0;
    size_t test = strlen(info->fileName) - 1;

    while (numTokensPrinted < numTokens || fileNameIndex <(strlen(info->fileName) - 1))
    {
        char lineBuffer[41] = {0};
        char fileNameBuffer[41] ={0};
        u16 tokensInBuffer = 1;

        if (numTokensPrinted < numTokens)
        {
            u16 totalLength = strlen(tokenPointer);
            if (totalLength > (sizeof(lineBuffer) - 1))
            {
                snprintf(lineBuffer, sizeof(lineBuffer), "%s", tokenPointer);
                tokenPointer += 40;
            }
            else
            {
                while (totalLength < (sizeof(lineBuffer) - 1) && (numTokensPrinted + tokensInBuffer) < numTokens)
                {
                    tokenPointer = stringQueue[tokensInBuffer + numTokensPrinted];
                    totalLength += strlen(tokenPointer) + 1;
                    ++tokensInBuffer;
                }
                if (totalLength > (sizeof(lineBuffer) - 1))
                {
                    --tokensInBuffer;
                    totalLength -= strlen(tokenPointer) + 1;
                }

                u8 lineBufferIndex = 0;
                for (u16 tokenIndex = 0; tokenIndex < tokensInBuffer; ++tokenIndex)
                {
                    i32 tokenLength = strlen(stringQueue[numTokensPrinted + tokenIndex]) % 40;
                    for (u8 c = 0; c < tokenLength; ++c)
                    {
                        lineBuffer[lineBufferIndex++] = stringQueue[numTokensPrinted + tokenIndex][c + (strlen(stringQueue[numTokensPrinted + tokenIndex]) / 40) * 40];
                    }   
                    if ((numTokensPrinted + tokenIndex) != (numTokens - 1) && tokenIndex != (tokensInBuffer - 1))
                    {
                        lineBuffer[lineBufferIndex++] = ' ';
                    }
                    if (totalLength == (sizeof(lineBuffer) - 1))
                    {
                        lineBuffer[sizeof(lineBuffer) - 1] = '\0';
                    }
                }
                numTokensPrinted += tokensInBuffer;
            }
        }

        for (int j = 0; j < 40 && info->fileName[fileNameIndex] != '\0'; ++j)
        {
            fileNameBuffer[j] = info->fileName[fileNameIndex];
            ++fileNameIndex;
        }

        char logText[LOG_PRINT_BUFFER_SIZE] = {0};
        if (firstLine)
        {
            char dateBuffer[DATE_BUFFER_SIZE] = {0}; 
            char dayBuffer[DAY_DATE_BUFFER_SIZE] = {0};
            char monthBuffer[MONTH_DATE_BUFFER_SIZE] = {0};
            
            char timeBuffer[TIME_BUFFER_SIZE] = {0};
            char hourBuffer[HOUR_BUFFER_SIZE] = {0};
            char minuteBuffer[MINUTE_BUFFER_SIZE] = {0};

            if (info->time.wDay < 10)
            {
                snprintf(dayBuffer, sizeof(dayBuffer), "0%d", info->time.wDay);
            }
            else 
            {
                snprintf(dayBuffer, sizeof(dayBuffer), "%d", info->time.wDay);
            }
            if (info->time.wMonth < 10)
            {
                snprintf(monthBuffer, sizeof(monthBuffer), "0%d", info->time.wMonth);
            }
            else 
            {
                snprintf(monthBuffer, sizeof(monthBuffer), "%d", info->time.wMonth);
            }
            snprintf(dateBuffer, sizeof(dateBuffer), "%s/%s/%d", dayBuffer, monthBuffer, info->time.wYear);
            
            if (info->time.wMinute < 10 )
            {
                snprintf(minuteBuffer, sizeof(minuteBuffer), "0%d", info->time.wMinute);
            }
            else
            {
                snprintf(minuteBuffer, sizeof(minuteBuffer), "%d", info->time.wMinute);
            }
            if (info->time.wHour < 10)
            {
                snprintf(hourBuffer, sizeof(hourBuffer), "0%d", info->time.wHour);
            }
            else 
            {
                snprintf(hourBuffer, sizeof(hourBuffer), "%d", info->time.wHour);
            }
            snprintf(timeBuffer, sizeof(timeBuffer), "%s:%s", hourBuffer, minuteBuffer);

            snprintf(logText, sizeof(logText), "|%-40s|%-10s|%-5s|%-40s|%-256s|\n", lineBuffer, dateBuffer, timeBuffer, fileNameBuffer, info->directory);
            DWORD logBytesWritten = 0;
            WriteFile(logFileHandle, logText, strlen(logText), &logBytesWritten, NULL);
            firstLine = 0;
        }
        else
        {
            snprintf(logText, sizeof(logText), "|%-40s|%-10s|%-5s|%-40s|%-256s|\n", lineBuffer, "", "", fileNameBuffer, "");
            DWORD logBytesWritten = 0;
            WriteFile(logFileHandle, logText, strlen(logText), &logBytesWritten, NULL);
        }

    }
    printLogLine(logFileHandle);
    HeapFree(heapHandle, 0, tokens);
}


void printLogLine(HANDLE logFileHandle)
{
    char lineArray[LOG_PRINT_BUFFER_SIZE] = {0};
    memset(lineArray, '-', sizeof(lineArray));
    DWORD bytesWritten = 0;
    lineArray[sizeof(lineArray) - 2] = '\n';
    lineArray[sizeof(lineArray) - 1] = '\0';
    WriteFile(logFileHandle, lineArray, strlen(lineArray), &bytesWritten, NULL);
    return;
}

logInfo createLogInfo(char *message, char *directory, char *fileName)
{
    logInfo info = {0};
    info.message = message;
    info.directory = directory;
    info.fileName = fileName;
    GetLocalTime(&info.time);
    return info;
}

i8 clearStdin(char *buffer, size_t bufferSize)
{
    if (bufferSize < 2)
    {
        return -1;
    }
    i32 c;
    while (!(buffer[bufferSize - 2] == '\n' || buffer[bufferSize - 2] == '\0')  && (c = getchar()) != '\n' && c != EOF)
        ;
    return 0;
}
