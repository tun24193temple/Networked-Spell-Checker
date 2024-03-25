#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>

#define SERVER_PORT 8080
#define MAX_WORD_LENGTH 100
#define DEFAULT_CONCURRENT_THREADS 10

//arguments
typedef struct{
    int isActive; //flag 
    const char* serverIp; //IP
}WorkerThreadArgs;

const char* getRandomWord();
void logActivity(pthread_t threadId, int socketId, const char* request, const char* response, time_t requestTime, time_t responseTime);
void *spellCheckThread(void *args);

const char* getRandomWord(){
    //list of words
    const char* wordList[] = 
        {"word", "wurd", "inserection", "insurrection", "art", "star",
        "zoo", "computer", "shaman", "door", "programming", "vector", 
        "detector", "thread", "synchronization"
    };

    //pick random word out of list
    int wordCount = 15;
    int index = rand() % wordCount;
    return wordList[index];
}

void addToLog(pthread_t threadId, int socketId, const char* request, const char* response, time_t requestTime, time_t responseTime){
    //convert time to string
    char *reqTimeStr = ctime(&requestTime);
    char *respTimeStr = ctime(&responseTime);

    //make log message
    char logMessage[1024];
    snprintf(logMessage, sizeof(logMessage), "Thread ID: %ld, Socket ID: %d, Request: %s, Response: %s, Request Time: %s, Response Time: %s\n",
             (long)threadId, socketId, request, response, reqTimeStr, respTimeStr);
    
    //remove '\n' from ctime output
    logMessage[strcspn(logMessage, "\n")] = ' ';
    logMessage[strrchr(logMessage, '\n') - logMessage] = ' ';

    //write log message to file
    FILE* logFile = fopen("client_log.txt", "a");
    if(logFile == NULL){
        perror("Error opening log file");
        return;
    }

    fprintf(logFile, "%s", logMessage);
    fclose(logFile);
}


void *spellCheckThread(void *arg){
    WorkerThreadArgs *args = (WorkerThreadArgs *)arg;
    args->isActive = 1;
    int sock;
    struct sockaddr_in server_addr;
    char word[MAX_WORD_LENGTH];
    char response[MAX_WORD_LENGTH];
    time_t requestTime, responseTime;

    //create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock == -1){
        perror("Socket creation failed");
        pthread_exit(NULL);
    }

    //configure server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(args->serverIp);

    //connect to server
    if(connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
        perror("Connection failed");
        close(sock);
        pthread_exit(NULL);
    }

    //select a random word for spell checking
    strcpy(word, getRandomWord());

    //log time of request
    requestTime = time(NULL);

    //send word to server
    if(send(sock, word, strlen(word) + 1, 0) < 0){
        perror("Send failed");
        close(sock);
        pthread_exit(NULL);
    }

    //receive response from server
    if(recv(sock, response, sizeof(response), 0) < 0){
        perror("Receive failed");
        close(sock);
        pthread_exit(NULL);
    }

    //log time of response
    responseTime = time(NULL);

    //log the activity
    addToLog(pthread_self(), sock, word, response, requestTime, responseTime);
    
    //close socket
    close(sock);

    args->isActive = 0;
    return NULL;
}


int main(int argc, char *argv[]){
    //provide 127.0.0.1 10 as arguments
    if(argc != 3){
        fprintf(stderr, "Usage: %s <Server IP> <Minimum Concurrent Threads>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *serverIp = argv[1]; //server IP
    int minThreads = atoi(argv[2]); //num threads
    if(minThreads < DEFAULT_CONCURRENT_THREADS){
        minThreads = DEFAULT_CONCURRENT_THREADS;
    }

    srand(time(NULL));

    //allocate me for thread args and threads
    WorkerThreadArgs *threadArgs = malloc(minThreads * sizeof(WorkerThreadArgs));
    pthread_t *threads = malloc(minThreads * sizeof(pthread_t));

    //handle malloc error
    if(threadArgs == NULL || threads == NULL){
        perror("Failed to allocate memory for threads or thread arguments");
        exit(EXIT_FAILURE);
    }

    //init thread args
    for(int i = 0; i < minThreads; ++i){
        threadArgs[i].isActive = 0; //set thread not active
        threadArgs[i].serverIp = serverIp; //assign serverIP to threads
    }

    //maintain atleast 10 threads at all times
    while(1){
        for(int i = 0; i < minThreads; ++i){
            //check if thread is active
            if(!threadArgs[i].isActive){
                //create new thread and pass its arguments 
                if(pthread_create(&threads[i], NULL, spellCheckThread, &threadArgs[i]) != 0){
                    perror("Failed to create a thread");
                }else{
                    threadArgs[i].isActive = 1;
                }
            }
        }
        usleep(1000000); 
    }

    free(threads);
    free(threadArgs);

    return 0;
}