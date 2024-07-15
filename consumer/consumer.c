#include <windows.h>
#include <stdio.h>
#include "common_resource.h"

#define MAX_THREADS 4

HANDLE hPipe;
HANDLE mutex;
HANDLE threads[MAX_THREADS];
DWORD bytesRead, bytesWritten;
ResultStruct sumArrayShared[NUM_RANDOM_NUMBERS];
ProducerMessage arr[NUM_RANDOM_NUMBERS];//defining array

//Forward declaration
void threadFunc();
DWORD WINAPI performArithmetic(LPVOID lpParam);
int pipeNameCreate();
int getDataFromProd();
int sendDataToProd();

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
        PIPE_NAME,
        PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
        PIPE_UNLIMITED_INSTANCES,
        BUFFER_SIZE,
        BUFFER_SIZE,
        0,
        NULL
    );

    if (hPipe == INVALID_HANDLE_VALUE) {
        printf("CreateNamedPipe failed, error code: %d\n", GetLastError());
        return 1;
    }

    return 0;
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
                printf("Client disconnected.\n");
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

    // int thread_args1[2] = { 0, NUM_RANDOM_NUMBERS / 2 };
    // int thread_args2[2] = { NUM_RANDOM_NUMBERS / 2 , NUM_RANDOM_NUMBERS,};

    mutex = CreateMutex(NULL, FALSE, NULL);

    for (int i = 0; i < MAX_THREADS; i++) {
        start_idx = i * (NUM_RANDOM_NUMBERS / MAX_THREADS);
        end_idx = start_idx + (NUM_RANDOM_NUMBERS / MAX_THREADS);
        int thread_args[2] = {start_idx, end_idx};
        threads[i] = CreateThread(NULL, 0, performArithmetic, thread_args, 0, NULL);
        Sleep(1);
    }
    // threads[0] = CreateThread(NULL, 0, performArithmetic, thread_args1, 0, NULL);
    // threads[1] = CreateThread(NULL, 0, performArithmetic, thread_args2, 0, NULL);

    //WaitForMultipleObjects(2, threads, TRUE, INFINITE);
    WaitForMultipleObjects(MAX_THREADS, threads, TRUE, INFINITE);
    CloseHandle(mutex);


    for (int i = 0; i < MAX_THREADS; i++) {
        CloseHandle(threads[i]);
    }
    // CloseHandle(threads[0]);
    // CloseHandle(threads[1]);
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
    return 0;
}

int sendDataToProd(){
    BOOL success;

    // Send modified data back to the producer
    for (int i = 0; i < NUM_RANDOM_NUMBERS; ++i) {
        success = WriteFile(
            hPipe,                 // Handle to pipe
            &sumArrayShared[i],                // Buffer to write from
            sizeof(sumArrayShared[i]),    // Number of bytes to write (include null terminator)
            &bytesWritten,         // Number of bytes written
            NULL);                 // Not overlapped I/O
    
        if (!success) {
            printf("Error writing to pipe: %d\n", GetLastError());
            CloseHandle(hPipe);
            return 1;
        }
    }

    printf("Sent to producer\n");
    return 0;

}
 
