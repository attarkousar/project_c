#include <windows.h>
#include <stdio.h>

typedef struct {
    int id;
    float x;
    float y;
    int command;
} ProducerMessage;

typedef struct {
    int id;
    float result;
} ResultStruct;

#define PIPE_NAME "\\\\.\\pipe\\MyNamedPipe"
#define BUFFER_SIZE sizeof(ProducerMessage)  // Use size of integer for buffer size
#define NUM_RANDOM_NUMBERS 4
ProducerMessage arr[NUM_RANDOM_NUMBERS];//defining array 


ResultStruct sumArrayShared[NUM_RANDOM_NUMBERS];
float sumArray1[NUM_RANDOM_NUMBERS/2];
float sumArray2[NUM_RANDOM_NUMBERS/2];
CRITICAL_SECTION cs;

HANDLE hPipe;


void ThreadFunc();
DWORD WINAPI AddNumbers(LPVOID lpParam);
int PipeNameCreate();
int GetDataFromProd();

int main() {
    if (PipeNameCreate() != 0) {
        exit(EXIT_FAILURE);
    }
    if (GetDataFromProd(hPipe) != 0) {
        exit(EXIT_FAILURE);
    }
    ThreadFunc();
    CloseHandle(hPipe);
    return 0;
}

int PipeNameCreate() {
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

int GetDataFromProd() {
    
    DWORD bytesRead, bytesWritten;
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
}
void ThreadFunc() {
    /////////////////////////////////////////////////////////////////////////// Threads ///////////////////////////////////////////////////////////////////////////////////
    HANDLE threads[2];
    int thread_args1[3] = { 0, NUM_RANDOM_NUMBERS / 2 , 1};
    int thread_args2[3] = { NUM_RANDOM_NUMBERS / 2, NUM_RANDOM_NUMBERS, 2 };
    
   InitializeCriticalSection(&cs);
    threads[0] = CreateThread(NULL, 0, AddNumbers, thread_args1, 0, NULL);
    threads[1] = CreateThread(NULL, 0, AddNumbers, thread_args2, 0, NULL);

    WaitForMultipleObjects(2, threads, TRUE, INFINITE);

    DeleteCriticalSection(&cs);

    CloseHandle(threads[0]);
    CloseHandle(threads[1]);
    for(int i=0; i< (NUM_RANDOM_NUMBERS); i++){
        printf("Sumarray [%.2f] id: %d Result: %.2f\n",i, sumArrayShared[i].id, sumArrayShared[i].result);
    }

}
typedef enum {
    COMMAND_ADD,
    COMMAND_SUB,
    COMMAND_MULT,
    COMMAND_DIV
} CommandType;

float processMessageCommands(ProducerMessage message)
{
    switch(message.command) {
        case COMMAND_ADD:
            return message.x + message.y;
        case COMMAND_SUB:
            return message.x - message.y;
        case COMMAND_MULT:
            return message.x * message.y;
        case COMMAND_DIV:
            if(message.y == 0) {
                message.y += 0.1; //Avoid zero div
            }
            return message.x / message.y;
        default:
            return 0;
    }
}

DWORD WINAPI AddNumbers(LPVOID lpParam) {
    int* args = (int*)lpParam;
    int start = args[0];
    int end = args[1];
    int idx = args[2];  

    for (int i = start; i < end; i++) {
        EnterCriticalSection(&cs);
        sumArrayShared[i].id = i;
       // sumArrayShared[i].result = arr[i].x + arr[i].y;
        sumArrayShared[i].result = processMessageCommands(arr[i]);
                
        LeaveCriticalSection(&cs);
    }

    return 0;
}
