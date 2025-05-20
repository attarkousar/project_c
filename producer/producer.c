#include <windows.h>
#include <stdio.h>
#include "common_resource.h" // Custom header file
#include <stdlib.h>  // For rand() and srand()
#include <time.h>    // For time()

HANDLE hPipe;
DWORD bytesRead, bytesWritten;
ResultStruct sumArrayShared[NUM_RANDOM_NUMBERS];
ResultStruct expectedResult[NUM_RANDOM_NUMBERS];

//Forward declaration
int pipeFileCreate();//Function to connect to the named pipe
int dataSentToCons();//Function to generate random messages, send them to the consumer process, and calculate expected results for checksum.
int dataRecievedFromCons();//Function to receive modified data from the consumer process
void checkCheckSum();//Function to verify checksum between expected and received data.

int main() {
    if (pipeFileCreate() != 0) {
        exit(EXIT_FAILURE);
    }
    DWORD startTime = GetTickCount(); // Get current time in milliseconds
    printf("Sending Data to Consumer...\n");
    if (dataSentToCons(hPipe) != 0) {
        exit(EXIT_FAILURE);
    }
    printf("Sent Data to Consumer...\n");
    printf("Receiving Data from Consumer...\n");
    if (dataRecievedFromCons(hPipe) != 0) {
        exit(EXIT_FAILURE);
    }
    printf("Received Data from Consumer...\n");
    DWORD endTime = GetTickCount(); // Get current time after the delay
    DWORD elapsedTime = endTime - startTime; // Calculate elapsed time in milliseconds
    printf("Elapsed time: %u milliseconds\n", elapsedTime);
    printf("Checking checksum...\n");
    checkCheckSum();

    // Close the pipe
    CloseHandle(hPipe);
    return 0;
}

void checkCheckSum() {
    boolean is_checksum_success = TRUE;
    for (int i = 0; i < NUM_RANDOM_NUMBERS; ++i) {
        if(expectedResult[i].id != sumArrayShared[i].id ||
           expectedResult[i].result != sumArrayShared[i].result) {
            printf("Checksum failed!\n");
            is_checksum_success= FALSE;
            break;
        }
    } 
    if (is_checksum_success){
        printf("Checksum Successful!\n");
    }
}

int pipeFileCreate() {
    // Connect to the named pipe
    hPipe = CreateFile(
        PIPE_NAME,                    // Pipe name
        GENERIC_READ | GENERIC_WRITE, // Access mode (read and write)
        0,                            // No sharing (pipe handle is not inherited)
        NULL,                         // Default security attributes
        OPEN_EXISTING,                // Opens an existing pipe
        0,                            // Default attributes and flags
        NULL                          // Template file (not used for pipes)
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

        // Wait a short time to simulate slower production of numbers
        Sleep(2);
    }
    return 0;
}

int dataRecievedFromCons(){
    BOOL success;
    for (int i = 0; i < NUM_RANDOM_NUMBERS; ++i) {
        // Read modified data from the consumer
        success = ReadFile(
            hPipe,                     // Handle to pipe
            &sumArrayShared[i],        // Buffer to receive data
            sizeof(sumArrayShared[i]), // Size of buffer
            &bytesRead,                // Number of bytes read
            NULL);                     // Not overlapped I/O

        if (!success || bytesRead == 0) {
            printf("Error reading from pipe: %d\n", GetLastError());
            CloseHandle(hPipe);
            return 1;
        }
    }
    return 0;
}