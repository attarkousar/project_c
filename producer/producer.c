#include <windows.h>
#include <stdio.h>
#include "myheader.h" // Custom header file
#include <stdlib.h>  // For rand() and srand()
#include <time.h>    // For time()

HANDLE hPipe;
DWORD bytesRead, bytesWritten;
ResultStruct sumArrayShared[NUM_RANDOM_NUMBERS];
ResultStruct expectedResult[NUM_RANDOM_NUMBERS];

//Forward declaration
int pipeFileCreate();
int dataSentToCons();
int dataRecievedFromCons();

int main() {
    // PipeFileCreate();
    if (pipeFileCreate() != 0) {
        exit(EXIT_FAILURE);
    }

    // DataSentToCons();
    if (dataSentToCons(hPipe) != 0) {
        exit(EXIT_FAILURE);
    }

    //DataRecievedFromCons();
    if (dataRecievedFromCons(hPipe) != 0) {
        exit(EXIT_FAILURE);
    }

    // Close the pipe
    CloseHandle(hPipe);
    return 0;
}

int pipeFileCreate(){
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
    return 0;
}

int dataSentToCons(){
    BOOL result;

    // Generate random numbers and send them one by one
    srand((unsigned int)time(NULL));

    for (int i = 0; i < NUM_RANDOM_NUMBERS; ++i) {
        ProducerMessage message;
        
        message.id      = i;
        message.x       = (float) (rand() % 100);
        message.y       = (float)(rand() % 100);
        message.command = rand() % 4;
        // Calculating expected result for checksum and storing it in array
        expectedResult[i].id = i;
        expectedResult[i].result = processMessageCommands(message);
        
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
    return 0;
}


int dataRecievedFromCons(){
    BOOL success;
    for (int i = 0; i < NUM_RANDOM_NUMBERS; ++i) {
        // Read modified data from the consumer
        success = ReadFile(
            hPipe,                 // Handle to pipe
            &sumArrayShared[i],                // Buffer to receive data
            sizeof(sumArrayShared[i]),           // Size of buffer
            &bytesRead,            // Number of bytes read
            NULL);                 // Not overlapped I/O

        if (!success || bytesRead == 0) {
            printf("Error reading from pipe: %d\n", GetLastError());
            CloseHandle(hPipe);
            return 1;
        }
        printf("Received from consumer. idx [%d], result: %.2f\n",sumArrayShared[i].id, sumArrayShared[i].result);
    }

   //checksum
    // for (int i = 0; i < NUM_RANDOM_NUMBERS; ++i) {
    //     if(expectedResult[i].id == sumArrayShared[i].id &&
    //         expectedResult[i].result == sumArrayShared[i].result) {
    //         printf("Checksum successful!\n");
    //     }
    //     else{
    //         printf("Checksum failed!\n");
    //     }
    for (int i = 0; i < NUM_RANDOM_NUMBERS; ++i) {
        if(expectedResult[i].id != sumArrayShared[i].id ||
           expectedResult[i].result != sumArrayShared[i].result) {
            printf("Checksum failed!\n");
        }
    }  
    return 0;
}