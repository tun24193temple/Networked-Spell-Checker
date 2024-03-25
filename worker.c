/*while (true) {
   while (the connection queue is NOT empty) {
      remove a socket data element from the connection buffer
      notify that there's an empty spot in the connection buffer
      service client
      close socket
   }
}

and the client servicing logic is:

while (there's a word left to read) {
   read word from the socket
   if (the word is in the dictionary) {
      echo the word back on the socket concatenated with "OK";
   } else {
      echo the word back on the socket concatenated with "MISSPELLED";
   }

   write the word, the socket response value (“OK” or “MISSPELLED”) and other log information to the log queue;

}*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h> 
#include <netinet/in.h> 
#include "worker.h"
#include "connectionBuffer.h"
#include "logManager.h"
#include "dictionary.h"

//service client
void serviceClient(int socket, LogBuffer *logBuff, ConnData data){
    while(1){
        //get word from client
        char word[MAX_WORD_LENGTH];

        int bytesReceived = recv(socket, word, sizeof(word), 0);
        if(bytesReceived <= 0){
            break;
        }

        word[bytesReceived] = '\0';

        //check spelling
        bool inDict = isWordInDictionary(word);

        //send response to client
        char response[MAX_WORD_LENGTH];
        if(inDict){
            strcpy(response, "OK");
        }else{
            strcpy(response, "MISSPELLED");
        }

        //when there is nothing else to write, break
        ssize_t bytesWritten  = write(socket, response, strlen(response) + 1);
        if(bytesWritten <= 0){
            break;
        }

        //create entry for log
        createLogEntry(logBuff, socket, word, response, data.arrivalTime, data.priority);
    }

    close(socket); 
}

//worker thread
void *workerThread(void *args){
    WorkerThreadArgs *workerArgs = (WorkerThreadArgs *)args;
    ConnBuffer *connBuff = workerArgs->connectionBuffer;
    LogBuffer *logBuff = workerArgs->logBuffer;

    while(1){
        //get data about current connection
        ConnData connDate = getConnData(connBuff);

        //if no connection keep waiting
        if(connDate.socket == 0){ 
            continue;
        }

        //when client connects, service them
        printf("Servecing client: %d\n", connDate.socket);
        serviceClient(connDate.socket, logBuff, connDate);
    }

    return NULL;
}


