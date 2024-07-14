#ifndef MYHEADER_H
#define MYHEADER_H


// Include necessary system headers
#ifdef _WIN32
#include <windows.h>  // Include Windows-specific headers for HANDLE and DWORD
#endif

// declaring Common resources
#define PIPE_NAME "\\\\.\\pipe\\MyNamedPipe"
#define BUFFER_SIZE sizeof(ProducerMessage)  // Use size of integer for buffer size
#define NUM_RANDOM_NUMBERS 4



// Define a struct for a producer message
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



typedef enum {
    COMMAND_ADD,
    COMMAND_SUB,
    COMMAND_MULT,
    COMMAND_DIV
} CommandType;

float processMessageCommands(ProducerMessage message);

#endif // MYHEADER_H
