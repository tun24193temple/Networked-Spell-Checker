<p align="center">
  <a href="" rel="noopener">
 <img width=200px height=200px src="https://i.imgur.com/6wj0hh6.jpg" alt="Project logo"></a>
</p>

<h3 align="center">Networked Spell Checker</h3>

---

<p align="center"> Few lines describing your project.
    <br> 
</p>

## 📝 Table of Contents
- [About](#about)
- [Getting Started](#getting_started)
- [Implementation](#implementation)
- [Testing](#testing)
- [Usage](#usage)

## 🧐 About <a name = "about"></a>
The purpose of this project is to gain additional exposure and practical experience with multi-threaded programming and the synchronization problems that go along with it. Also, I want to learn about networks and socket programming and writing programs that communicate across networks using sockets for communication.

## 🏁 Getting Started <a name = "getting_started"></a>
To familiarize myself with socket programming and how a server connects with clients, I read the resources provided by the instructor, the main one being Bryant O'Halloren chapter 11, which provided most of my server initialization logic. I also made a diagram consisting of the server and clients and how they would be connecting and what resources they would be using for this specific. I understood that this project is split up into several concurrent programs consisting of a connection buffer to store client connection's to the server, a logging mechanism to report those connections and save them into a txt file and worker logic that provides the spell-checking service of the server. I realized this when I first started setting up the main function of the server. Since it was similar to the project 2 all i did was set up connections and create threads in this server. The thread logic will be where I will be implementing most of my logic, and since the only two threads I created was a log manager thread and a few worker threads, I would start with developing the worker thread.
### Worker.c Beginning:
The project provided me with pseudo code for the worker thread which I used as the actual coding scheme of the worker thread function. I decided with this approach to help me organize the code better into several helper functions. Within the pseudo-code, I realized that there were several aspects of the worker thread and I will be developing this project around the functionality of the worker thread. The worker thread required a connection, so we had to create a connection buffer first. Next, it would service this client which I set up as a seperate function. Within it, I realized I needed a seperate function to load the dictionary as well as a function to check if the word was correct. Servicing the client also required an entry to the log which I will make after finishing the dictionary and connection buffer logic.

## Implementation <a name = "implementation"></a>
### Dictionary.c:
1. loadDictionary(): Takes no parameters and simply loads the linux dictionary at /usr/share/dict/words into a global array which I will store all the words in the dictionary
2. isWordInDictionary(const char *word): Takes a string as an argument and compares it to all words stored in my dictionary array. It returns true when it finds a match and false when it doesnt
### ConnectionBuffer.c:
#### Structs:
I created two structs, one that holds data about the connections and a struct representing the connection buffer that holds these connections implemented as a circular queue
1. Struct ConnData{socket, arrivalTime, priority}
2. Struct ConnBuffer{mutex, emptyCondition, fullCondition, *buffer, first, last, count, size}
#### Logic:
1. initConnBuffer(ConnBuffer *buffer, int bufferSize): Initializing function for the connection Buffer.
2. addConnData(ConnBuffer *buffer, ConnData data): Simple adding function which waits for empty space on the buffer and adds the connection data to the last spot available in the buffer and indexes last to the next spot in the circular queue.
3. ConnData getConnData(ConnBuffer *buffer): Function that worker.c will use to remove the connection with the current highest priority for processing. It does this by looping the buffer for the connection with the highest priority and swapping the connection with the highest priority with the connection in the first spot in the connection queue.
### LogManager.c:
#### Struct:
Similar to the connection buffer struct I created a log Buffer that stores log entries.
Struct LogBuffer{mutex, empty, full, buffer[size], first, last, count}
#### Logic:
1. initLogBuffer(LogBuffer *logBuffer)
2. addLogEntry(LogBuffer *logBuffer, const char *entry): simple circular queue addition into the buffer similar to connectionBuffer's add function.
3. createLogEntry(...): takes several parameters about the information of the log and puts those arguments into a logEntry in a readable char format which is used by the addLogEntry() function.
4. logManagerFunction(args): main thread function for the log Manager. it removes an entry from the buffer and appends it to the log.txt file in the server directory
### Worker.c:
1. serviceClient(int socket, LogBuffer *logBuffer, ConnData data): recieves a word from the client usinf recv() and stores it into an array, word[]. Checks if word is spelled correct using isWordInDictionary(word). Create a response array and if the word is correct, write ok into the array and misspelled if not. Write response to the socket and create a log entry. close the socket.
2. workerThread(args): Takes connection buffer and log buffer as arguments. Inside an infinite loop, get connection from the buffer and service the client.
### Server.c: 
Hosts the main function for the server. first is loads the dictionary and initializes the connection and log buffers. It then creates 4 worker threads and a log manager thread. Then we assign a socket to the server. Bind the server socket to the port and start listening to incoming connections. The main loop accepts a client socket and assigns its data to a ConnData variable and then adds that connection to the connection buffer.

## 🔧 Testing <a name = "testing"></a>
### Testing Option Parsing Function
#### Testing Dictionary.c
1. Code for testing the upload of the linux dictionary
* ![Main](Images/proj3_dict_test_code.png)
2. Output
* ![Output](Images/proj3_dict_test.png)
#### Testing ConnectionBuffer.c
1. Code for testing if connection buffer is adding things in correctly and also retrieving connections based on priority:
* ![Main](Images/proj3_conn_buffer_test_main.png)
2. Output
* ![Output](Images/proj3_conn_buffer_test_output.png)
#### Testing LogManager.c
1. Code for testing if log manager is creating and adding to the log buffer a txt file
* ![Functions](Images/proj3_log_manager_test_main.png)
2. Output:
* ![Output](Images/proj3_log_manager_test_output.png)
* ![TXT](Images/proj3_log_manager_test_txt.png)
#### Testing Worker.c
1. Mostly had print statements around the code while testing which kept track of errors if they occurred. Pictures below are from right after my first working run of he project
* ![Output](Images/proj3_worker_test_prints.png)
* ![Output](Images/proj3_worker_serviceCLient_test_code.png)
2. Output:
* ![Output](Images/proj3_worker_output.png)

### Frequent Errors While Testing
1. My first implementation of the connection buffer is not what it ended up being. In the beginning, it was much more complicated because it did not only check for priority levels but also had a conditional that recorded the time of each entry in the connection buffer, and if there where more than one connection with the same highest priority value, then the one that came first with be serviced first. I opted to get rid of it because it became very unreadable and I kept having problems compiling with the -Wall -Werror flags. After I successfully compiled it, it would not correctly service the clients. Additionally, I thought there was no need to do such a thing since I could implement the queue as a circular FIFO queue, so the first highest priority it reads will go to the front anyway by swaping it with the connection on 'first'.
2. Another problem with the connection buffer and main function was the sequential processing. For some reason, I could not get it to work as an option when I started running my server and it was originally a member of my ConnBuffer struct. However, whenever I inputted the argument for main to switch the isSequential member of the struct from false to true, it ignored my conditional and just defaulted to the priority processing. I decided to ignore the problem and move on to implement the rest of the project but ran out of time when I went back to try and fix it and decided to scrap the implementation and lose a couple of points.
3. The next issue I ran into was within the servicing function of the worker.c file. For a while, there was an issue in my code that made it impossible for the program to compare the words I would input with the words in my dictionary. Any word I sent would get a response of "MISSPELLED". I hate to admit it but I spent a couple of hours only to figure out that I had forgotten to null terminate the string the server receives from the client. However, it still would not work. After looking at the way the words are stored in the linux dictionary, I noticed they where seperated by a '/n' character. In my loadDictionary function, I added a line of code that removed this character from my stored words. The next test worked as intended.
4. There where numerous problems with the server starting up through all my implementations. In the begging, the server would start and after a couple of runs, it would freeze in place. This issue was due to my first implementation of the connection buffer. I didn't implement it as a circular queue and had a seperate for loop that was signaled to start iterating through connections after the buffer was full and in this way it would be sequential. This implementation was too buggy and I opted for a circular queue instead. 
## 🎈 Usage <a name="usage"></a>
1. Install all files to your working environment and type make into the terminal which will compile your server under the executable 'server'
2. run ./server to start running the server.
3. Compile client script and run ./client 127.0.0.1 [number of client threads]
