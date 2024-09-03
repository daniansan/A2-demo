#ifndef __KEY_H_
#define __KEY_H_


#include "multiButton.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "fcntl.h"
#include "unistd.h"
#include "led.h"
#include "pthread.h"


typedef struct TKeyhandlerTab
{
    int fd;
}Keyhandler;

void Key_Init(int gpioNum,Keyhandler* key);
char Key_Status(Keyhandler* key);
void Key_close(Keyhandler* key);

#endif //__KEY_H_