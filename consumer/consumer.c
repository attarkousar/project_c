#include <windows.h>
#include <stdio.h>
#include "common_resource.h"

#define MAX_THREADS 8

HANDLE hPipe;
HANDLE mutex;
HANDLE threads[MAX_THREADS];
DWORD bytesRead, bytesWritten;
ResultStruct sumArrayShared[NUM_RANDOM_NUMBERS];
ProducerMessage arr[NUM_RANDOM_NUMBERS];//defining array

//Forward declaration
void threadFunc();//to distribute processing of received data among multiple threads.
DWORD WINAPI performArithmetic(LPVOID lpParam);
int pipeNameCreate();// to create a named pipe.
int getDataFromProd();//to receive data from the producer via the named pipe.
int sendDataToProd();//to send processed data back to the producer.

int main() {
    if (pipeNameCreate() != 0) {
        exit(EXIT_FAILURE);
    }
    printf("Receiving Data from Producer...\n");
    if (getDataFromProd(hPipe) != 0) {
        exit(EXIT_FAILURE);
    }
    printf("Processing data on multiple threads...\n");
    threadFunc();
    printf("Sending back processed data to producer...\n");
    if (sendDataToProd(hPipe) != 0) {
        exit(EXIT_FAILURE);
    }

    CloseHandle(hPipe);

    return 0;
}

int pipeNameCreate() {
    // Create a named pipe
    hPipe = CreateNamedPipe(
        PIPE_NAME,                              // Pipe name
        PIPE_ACCESS_DUPLEX,                     // Pipe open mode: read/write access
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, // Pipe mode and wait mode
        PIPE_UNLIMITED_INSTANCES,               // Max instances
        BUFFER_SIZE,                            // Output buffer size
        BUFFER_SIZE,                            // Input buffer size
        0,                                      // Default time-out (0 means default)
        NULL                                    // Default security attributes
    );

    if (hPipe == INVALID_HANDLE_VALUE) {
        printf("CreateNamedPipe failed, error code: %d\n", GetLastError());
        return 1;                               // Return failure if pipe creation fails
    }

    return 0;                                   // Return success if pipe creation succeeds
}

int getDataFromProd() {
    BOOL result;

    printf("Waiting for producer to connect...\n");

    result = ConnectNamedPipe(hPipe, NULL);
    if (!result) {
        printf("ConnectNamedPipe failed, error code: %d\n", GetLastError());
        CloseHandle(hPipe);
        return 1;
    }
    printf("Producer connected. Waiting for messages...\n");

    int count = 0;//counter intialization
    while (1) {
        ProducerMessage message;
        // Read message from the pipe
        result = ReadFile(hPipe, &message, sizeof(message), &bytesRead, NULL);
        if (!result || bytesRead == 0) {
            DWORD dwError = GetLastError();
            if (dwError == ERROR_BROKEN_PIPE) {
                printf("Producer disconnected.\n");
            } else {
                printf("ReadFile failed, error code: %d\n", dwError);
            }
            break;
        }
        arr[count] = message;
        count++;
        if (count >= NUM_RANDOM_NUMBERS) {
            break;    
        }

    }
    return 0;
}

void threadFunc() { 
    int start_idx = 0;
    int end_idx   = 0;

    // Create a mutex for synchronizing access to shared array
    mutex = CreateMutex(NULL, FALSE, NULL);

    for (int i = 0; i < MAX_THREADS; i++) {
        start_idx = i * (NUM_RANDOM_NUMBERS / MAX_THREADS);
        end_idx   = start_idx + (NUM_RANDOM_NUMBERS / MAX_THREADS);

        // Allocate thread arguments dynamically
        int* thread_args = (int*)malloc(2 * sizeof(int));
        if (thread_args == NULL) {
            printf("Memory allocation failed for thread %d\n", i);
            continue; // skip this thread if malloc fails
        }

        thread_args[0] = start_idx;
        thread_args[1] = end_idx;

        // Create thread and pass allocated args
        threads[i] = CreateThread(
            NULL,            // default security
            0,               // default stack size
            performArithmetic,      // thread function
            thread_args,     // arguments
            0,               // run immediately
            NULL             // thread id
        ); 

        if (threads[i] == NULL) {
            printf("Failed to create thread %d. Error: %lu\n", i, GetLastError());
            free(thread_args); // cleanup if thread creation fails
        }

        Sleep(1); // slight delay to avoid race on arg pointer
    }

    // Wait for all threads to finish
    WaitForMultipleObjects(MAX_THREADS, threads, TRUE, INFINITE);

    // Cleanup
    CloseHandle(mutex);
    for (int i = 0; i < MAX_THREADS; i++) {
        if (threads[i] != NULL) {
            CloseHandle(threads[i]);
        }
    }
}


DWORD WINAPI performArithmetic(LPVOID lpParam) {
    int* args = (int*)lpParam;
    int start = args[0];
    int end = args[1];

    for (int i = start; i < end; i++) {
        WaitForSingleObject(mutex, INFINITE); 
        sumArrayShared[i].id = i;
        sumArrayShared[i].result = processMessageCommands(arr[i]);// sumArrayShared[i].result = arr[i].x + arr[i].y;
        ReleaseMutex(mutex);

        //Simulate Time consuming task as compared to arithmetic operations
        Sleep(100); //Strictly for making task execution time more than data transmision
        
    }
    // Free dynamically allocated args
    free(args);
    return 0;
}

int sendDataToProd(){
    BOOL success;

    // Send modified data back to the producer
    for (int i = 0; i < NUM_RANDOM_NUMBERS; ++i) {
        success = WriteFile(
            hPipe,                     // Handle to pipe
            &sumArrayShared[i],        // Buffer to write from
            sizeof(sumArrayShared[i]), // Number of bytes to write (include null terminator)
            &bytesWritten,             // Number of bytes written
            NULL);                     // Not overlapped I/O
    
        if (!success) {
            printf("Error writing to pipe: %d\n", GetLastError());
            CloseHandle(hPipe);
            return 1;
        }
    }

    printf("Sent to producer\n");
    return 0;

}
 
