#include <windows.h>
#include <stdio.h>
#include "myheader.h" // Custom header file

HANDLE hPipe;
DWORD bytesRead, bytesWritten;
ResultStruct sumArrayShared[NUM_RANDOM_NUMBERS];


ProducerMessage arr[NUM_RANDOM_NUMBERS];//defining array
float sumArray1[NUM_RANDOM_NUMBERS/2];
float sumArray2[NUM_RANDOM_NUMBERS/2];

//CRITICAL_SECTION cs;
HANDLE mutex;
HANDLE threads[2];

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
    if (getDataFromProd(hPipe) != 0) {
        exit(EXIT_FAILURE);
    }
    threadFunc();

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
    //DWORD bytesRead, bytesWritten;
    BOOL result;

    printf("Waiting for client to connect...\n");
    // Wait for a client to connect
    result = ConnectNamedPipe(hPipe, NULL);
    if (!result) {
        printf("ConnectNamedPipe failed, error code: %d\n", GetLastError());
        CloseHandle(hPipe);
        return 1;
    }

    printf("Client connected. Waiting for messages...\n");

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

        // Process the received integer
        printf("Received random number from client:id: %d, x: %.2f, y: %.2f, command: %d\n", message.id, message.x, message.y, message.command);
      
        arr[count] = message;
        count++;

        if (count >= NUM_RANDOM_NUMBERS) {
            break;    
        }
    }
    return 0;
}


void threadFunc() {
    
    int thread_args1[3] = { 0, NUM_RANDOM_NUMBERS / 2 , 1};
    int thread_args2[3] = { NUM_RANDOM_NUMBERS / 2 , NUM_RANDOM_NUMBERS, 2 };
    
    //InitializeCriticalSection(&cs);
    mutex = CreateMutex(NULL, FALSE, NULL);

    threads[0] = CreateThread(NULL, 0, performArithmetic, thread_args1, 0, NULL);
    threads[1] = CreateThread(NULL, 0, performArithmetic, thread_args2, 0, NULL);

    WaitForMultipleObjects(2, threads, TRUE, INFINITE);

    //DeleteCriticalSection(&cs);
    CloseHandle(mutex);
    for(int i=0; i< (NUM_RANDOM_NUMBERS); i++){
        printf("Sumarray [%d] id: %d Result: %.2f\n",i, sumArrayShared[i].id, sumArrayShared[i].result);
    }

    CloseHandle(threads[0]);
    CloseHandle(threads[1]);

}


DWORD WINAPI performArithmetic(LPVOID lpParam) {
    int* args = (int*)lpParam;
    int start = args[0];
    int end = args[1];
    int idx = args[2];  

    for (int i = start; i < end; i++) {
        //EnterCriticalSection(&cs);
        WaitForSingleObject(mutex, INFINITE); // Lock the mutex 
        // Access shared resource
        sumArrayShared[i].id = i;
       // sumArrayShared[i].result = arr[i].x + arr[i].y;
        sumArrayShared[i].result = processMessageCommands(arr[i]);
                
        //LeaveCriticalSection(&cs);
        ReleaseMutex(mutex); // Unlock the mutex
    }
    return 0;
}


int sendDataToProd(){
    // DWORD bytesRead, bytesWritten;
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

    //printf("Sent to producer: %d\n", sumArrayShared);// you can modify this to loop through array using for loop
    printf("Sent to producer\n");
    return 0;

}
 
