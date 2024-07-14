#include "myheader.h"
#include <windows.h>
#include <stdio.h>

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