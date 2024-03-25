/* A client-server transaction consists of
four steps:
    1. When a client needs service, it initiates a transaction by
        sending a request to the server. For example, when a Web
        browser needs a file, it sends a request to a Web server.

    2. The server receives the request, interprets it, and manipulates
        its resources in the appropriate way. For example, when a Web
        server receives a request from a browser, it reads a disk file.

    3. The server sends a response to the client and then waits for the
        next request. For example, a Web server sends the file back to
        a client. 
    4. The client receives the response and manipulates it. For
        example, after a Web browser receives a page from the server,
        it displays it on the screen.

A SOCKET is an end point of a connection. Each socket has a
corresponding socket address that consists of an Internet address and
a 16-bit integer port and is denoted by the notation address:port.

Client:           Server:
getaddrinfo         getaddrinfo
socket              socket
                    bind
                    listen                      
connect  ---------> accept <-------         
rio_written ------> rio_readlineb  | 
rio_readlineb <---- rio_writen     | 
close       ------> rio_readlineb  | 
                    close ---------
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <limits.h>

#include "dictionary.h"
#include "connectionBuffer.h"
#include "logManager.h"
#include "worker.h"


int main(int argc, char *argv[]){
    int port = 8080;

    //load dictionary
    loadDictionary();

    //init conn buffer
    ConnBuffer connbuff;
    initConnBuffer(&connbuff, 10);
    printf("Initiated connetion buffer\n");

    //init log buffer
    LogBuffer logBuff;
    initLogBuffer(&logBuff);
    printf("Initiated log buffer\n");

    //create worker threads
    pthread_t workerThreads[4];
    WorkerThreadArgs args = {&connbuff, &logBuff};

    for(int i = 0; i < 4; i++){
        pthread_create(&workerThreads[i], NULL, workerThread, (void *)&args);
    }

    //create log thread
    pthread_t logThread;
    pthread_create(&logThread, NULL, logManagerFunction, (void *)&logBuff);
    printf("Created threads\n");

    //start server
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serverAddr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port = htons(port)
    };

    if(bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0){
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }
    printf("Port binding\n");

    if(listen(serverSocket, 10) < 0){
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", port);

    while (true){
        int clientSocket = accept(serverSocket, NULL, NULL);
        if(clientSocket < 0){
            perror("Accept failed");
            continue;
        }

        //add a connection to the connection buffer
        ConnData data;
        data.socket = clientSocket;
        data.arrivalTime = time(NULL);
        data.priority = rand() % 10 + 1;

        addConnData(&connbuff, data);
        usleep(1000000);
    }

    close(serverSocket);
    return 0;
}
