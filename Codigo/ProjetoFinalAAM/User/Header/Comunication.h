#ifndef COMUNICATION_H
#define COMUNICATION_H

#include "debug.h"

void Comunication_Init(void);

void SendNewMessage(const char* message);

void SendData(void* data, uint16_t size);
u8 ReceiveData(char* data, uint16_t size);


#endif