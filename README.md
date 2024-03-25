# Project-3-f23
## Networked Spell Checker   
### Project Overview
Spell checkers are useful utility programs often bundled with text editors. In this assignment, you'll create a networked spell check server and evaluate its performance.  

### Project Objectives
•	Gain additional exposure and practical experience with multi-threaded programming and the synchronization problems that go along with it,   
•	Learn about networks and socket programming,  
•	Learn about and gain experience with writing programs that communicate across networks using sockets for communication.  

You'll learn a bit about network sockets in lecture and lab. Much more detailed information is available in Chapter 11 of Bryant and O'Hallaron, and Chapters 57-62 in Kerrisk (see Canvas Files: Additional Textbook References). [Beej's Guide](http://beej.us/guide/bgnet/) and [BinaryTides' Socket Programming Tutorial](http://www.binarytides.com/socket-programming-c-linux-tutorial/) are potentially useful online resources.  

The high-level view of network sockets is that they are communication channels between pairs of processes, somewhat like Linux pipes (A Linux pipe is a unidirectional data channel that can be used for communication between two processes. Communication through a pipe is enabled with file descriptors in the same way we communicate with files.). They differ from pipes in that a pair of processes communicating via a socket may reside on different machines, and that the channel of communication is bi-directional.  

Much of "socket programming" involves the mechanics of setting up the channel (i.e., the socket) for communication. Once this is done, we're left with a socket descriptor, which we use in much the same manner as we've used descriptors to represent files.  

In this project you will develop a server program that performs spell checking on demand. Your spell check server is to be a process that will acquire (read) sequences of words sent by clients. If a word is in the server’s dictionary, it's considered to be spelled properly. If not, it's considered to be misspelled. The dictionary itself is nothing but a very long list of words stored in plain text form. On cis-linux2, one is available for your use in /usr/share/dict/words. (There is a downloadable copy in the Canvas Files directory.) You are not required to do any more sophisticated matching; for example, recognizing that perhaps the word "derps", which is not in the dictionary, might be the plural of "derp", which is in the dictionary. (For the record, neither "derp" nor "derps" is in cis-linux2's dictionary, although the dictionary does seem to include many of the forms of many words.)  

### Server Program Operation
**Server Main Thread**  
Your server program should take as a command line several control parameters. The first parameter is the name of a dictionary file. If none is provided, DEFAULT_DICTIONARY is used (where DEFAULT_DICTIONARY is a named constant defined in your program). The program should also take as a parameter a “port number” on which to listen for incoming connections. Similarly, if no port number is provided, your program should listen on DEFAULT_PORT (defined in your program).  

Three other command line parameters are: the number of element cells in the ‘connection buffer’, the number of worker threads, and the scheduling type for spell checking. These parameters are discussed in this document.   

The server will have two primary functions: 1) accept and distribute connection requests from clients, and 2) construct a log file of all spell check activities.  

When the server starts, the main thread opens the dictionary file and reads it into some data structure accessible by all the threads in the program. It also creates a fixed-sized data structure (connection buffer) which will be used to store the socket descriptor information of the clients that will connect to the server. The number of elements in this data structure (shared buffer) is specified by a program input parameter.   

The main thread creates a pool of worker threads (the number of threads is specified as a program parameter), and then the main thread immediately begins to accept and distribute connection requests in the following manner:  
```
while (true) {  
   connected_socket = accept(listening_socket);  
   add connected socket information to the connection buffer;  
   signal any sleeping workers that there's a new socket connection in the buffer;  
}  
```
A second server thread will monitor a log queue and process entries by removing and writing them to a log file.  
```
while (true) {  
   while (the log queue is NOT empty) {  
       remove an entry from the log  
       write the entry to the log file  
      }  
 }  
```  


**Connection Buffer Data**  
The cells in the connection buffer are filled by the main server thread. The cells are processed by the worker threads. Each cell is to contain the connection socket, the time at which the connection socket was received, and the priority of the request.  

The priority of the request is specified by the server when the request is received by the server. The main server thread will randomly choose a priority between 1 (low) and 10 (high) for a priority value and assign it to this request. The priority value is for use in the second type of worker processing described below.  

**Worker Thread**  
A server worker thread's main loop is as follows:  
```
while (true) {  
   while (the connection queue is NOT empty) {  
      remove a socket data element from the connection buffer  
      notify that there's an empty spot in the connection buffer  
      service client  
      close socket  
   }  
}  
```
and the client servicing logic is:  
```
while (there's a word left to read) {  
   read word from the socket  
   if (the word is in the dictionary) {  
      echo the word back on the socket concatenated with "OK";  
   } else {  
      echo the word back on the socket concatenated with "MISSPELLED";  
   }  
   write the word, the socket response value (“OK” or “MISSPELLED”) and other log information to the log queue;  

}  
```

We quickly recognize this to be an instance of the Producer-Consumer Problem that we have studied in class. The work queue (connection buffer) is a shared data structure, with the main thread acting as a producer, adding socket descriptors to the queue, and the worker threads acting as consumers, removing socket descriptors from the queue. Similarly, the log queue is a shared data structure, with the worker threads acting as producers of results into the buffer and a server log thread acting as a consumer, removing results from the buffer. Because we have concurrent access to these shared data structures, we must synchronize access to them using the techniques that we've discussed in class so that: 1) each client is serviced, and 2) the queues do not become corrupted.  

**Spell Processing**  
One of the parameters for program control is the type of spell processing. There are two types. The first type is strict sequential processing of client requests in the shared buffer. In this mode, the server inserts socket descriptions into the buffer in the order received. Worker threads remove the socket descriptors in FIFO order from the buffer.  

In the second type of processing, a worker thread finds the highest priority request and removes and processes that client information from the queue.  

Once the spelling has been checked and the response sent to the client, the worker thread will create an entry in the log buffer. The log buffer entry is to contain the arrival time of the request, the time the spell check was completed, the word that was checked, the result of the spell check and the priority of the request.  

### Synchronization  
**Correctness**  
Only a single thread at a time may manipulate the work queue (connection buffer). We've seen that this can be guaranteed through the proper use of mutual exclusion. Your solution should include synchronization using locks and condition variables.  

No more than one worker thread at a time should manipulate the log queue at any one time. This is ensured through the proper use of mutual exclusion. Again, synchronization should be using locks and condition variables.  

**Efficiency**  
A producer should not attempt to produce if the queue is full, nor should consumers attempt to consume when the queue is empty. When a producer attempts to add to the queue and finds it full, it should cease to execute until notified by a consumer that there is space available.   

Similarly, when a consumer attempts to consume and finds the queue empty, it should cease to execute until a producer has notified it that a new item has been added to the queue. As we've seen in class, locks and condition variables can be used to achieve this. Your solution should not involve thread yields or sleeps.  

**The Dictionary**  
We need to be very careful about how we access the work queue, but what about the dictionary? Is the dictionary not a shared resource that is accessed concurrently? Does it not require protection?  

Once the dictionary is loaded into memory, it is only read by the worker threads, not modified, so we don't need to protect it with mutual exclusion.  

### Code Organization  
Concurrent programming is tricky. Don't make it any trickier than it needs to be. Bundle your synchronization primitives along with the data they're protecting into a struct, define functions that operate on the data using the bundled synchronization primitives, and access the data only through these functions. In the end, we have something in C that looks very much like the Java classes you've written in 1068 and 2168 with some "private" data and "public" methods, or like monitor functions. Code and some very good advice can be found in Bryant and O'Hallaron Chapter 12.  

**Testing your program**  
At the beginning, as you are developing your server, you'll probably run the server and a client program on your own computer. When doing this, your server's network address will be the loopback address of 127.0.0.1. (do some research on this).  

You may write a basic client to test your server, however, you are not required to submit one for the assignment (This will be covered in an add-on assignment). You could also use the Unix telnet client, which, in addition to running the telnet protocol, can be used to connect to any TCP port, or you could use a program such as netcat. You will need to use a client to test and demonstrate your solution.  

You're also welcome to use this very basic Python client.  

Once you're ready to deploy your program on a real network, please restrict yourself to the nodes on cis-linux2 (see below). Start an instance of your server on one of the cis-linux2 systems and run multiple simultaneous instances of a client on other systems.  

You should use many instances of clients requesting spell check services at the same time (for the demo, use of multiple clients is required). These clients should be run from more than 1 computer system simultaneously, i.e., each client computer system should run many client instances at the same time. Your testing and demonstration should show this.  

The server program has options for buffer size and number of worker threads as parameters. You must use variations in these parameters to demonstrate that you are able to ensure proper synchronization and performance under varying loads of requests.  

  |  cis-linux2 nodes|        |         |
  | ---------------- | ------ | ------- |
  |  cis-lclient01  |      cis-lclient05 |      cis-lclient11 |  
  |  cis-lclient02  |      cis-lclient06 |      cis-lclient12 |  
  |  cis-lclient04  |      cis-lclient07 |      cis-lclient15 |  

There will be **weekly deliverables** and they are to be submitted to Canvas as well.  

WEEK 1  
Create main server thread, worker threads and log manager thread.  
Create buffer and management for socket descriptor elements from main server thread  
Create management of socket descriptors by worker threads  
Create main thread socket for listening for incoming requests from clients  

Week 2  
Create client send request socket for communication with main server thread  
Create worker thread insertion in the log buffer  
Create worker thread communication with the client for spell checking  

Week 3  
Create worker thread spell checking  
Create log manager thread updates to log file  
Create client generation and processing of words to spell  
Testing with single and multiple clients and varied intervals of requests, buffer sizes and numbers of worker threads for spell check  
Project completion and submission for demo  
