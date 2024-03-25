#ifndef WORKER_H
#define WORKER_H

#include "connectionBuffer.h"
#include "logManager.h"

//struct to pass args to worker thread
typedef struct{
    ConnBuffer *connectionBuffer;
    LogBuffer *logBuffer;
}WorkerThreadArgs;

void serviceClient(int socket, LogBuffer *logBuff, ConnData data);
void *workerThread(void *args);

#endif 


