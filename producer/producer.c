#include <windows.h>
#include <stdio.h>
#include <stdlib.h>  // For rand() and srand()
#include <time.h>    // For time()

// Define a struct for a producer message
typedef struct {
    int id;
    float x;
    float y;
    int command;
} ProducerMessage;

#define PIPE_NAME "\\\\.\\pipe\\MyNamedPipe"
#define BUFFER_SIZE sizeof(ProducerMessage)  // Use size of integer for buffer size
#define NUM_RANDOM_NUMBERS 4



int main() {
    HANDLE hPipe;
 
    DWORD bytesRead, bytesWritten;
    BOOL result;

    // Connect to the named pipe
    hPipe = CreateFile(
        PIPE_NAME,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );

    if (hPipe == INVALID_HANDLE_VALUE) {
        printf("CreateFile failed, error code: %d\n", GetLastError());
        return 1;
    }

    // Generate random numbers and send them one by one
    srand((unsigned int)time(NULL));

    for (int i = 0; i < NUM_RANDOM_NUMBERS; ++i) {
        ProducerMessage message;
        
        message.id      = i;
        message.x       = (float) (rand() % 100);
        message.y       = (float)(rand() % 100);
        message.command = rand() % 4;

        // Send message to the server
        result = WriteFile(hPipe, &message, sizeof(message), &bytesWritten, NULL);
        if (!result) {
            printf("WriteFile failed, error code: %d\n", GetLastError());
            CloseHandle(hPipe);
            return 1;
        }

        printf("Sent random number to server\n");

        // Wait a short time to simulate slower production of numbers
        Sleep(10);
    }

    // Close the pipe
    CloseHandle(hPipe);
    return 0;
}
