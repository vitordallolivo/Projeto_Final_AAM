#ifndef COMUNICATION_H
#define COMUNICATION_H

#include "debug.h"
#include "../Header/ThrustManager.h"

void Comunication_Init(void);
void SendNewMessage(const char* message);
void SendData(void* data, uint16_t size);

void ProcessReceivedData(void);
void GetInputData(Input_data *data);

u8 ReceiveData(char* data, uint16_t size);

#endif