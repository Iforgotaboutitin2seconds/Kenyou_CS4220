Kenyou Teoh, Emiliano Chavez De La Torre, Emmanuel Chavez De La Torre
We/I have neither given nor received unauthorized assistance on this work

----------------------------------------------------------------------------

How to build/run my program:

Step 1: Places server.c and client.c in different folder.

build server.c:
gcc server.c -o server -std=c99
build client.c:
gcc client.c -o server -std=c99
build message.txt:
just make a message.txt file with messsage you want to send

Step 2: Run it with 2 terminals

Terminals 1 run server:
./server
and just leave it be

Terminals 2 run client:
./client
Enter file name: message.txt

You should be able to see the results.

----------------------------------------------------------------------------
Question 1 how do I make a UDP server?
After some research I figure that TCP and UDP is simllar and different. That:
In terms of server and client architecture, a TCP server must accept connections from clients and often communicates with them over long-lived connections. A UDP server, on the other hand, listens for incoming datagrams from any number of clients and often processes each one independently, without needing to maintain a continuous connection. Clients using TCP need to establish a connection before sending data to the server, while UDP clients can just send datagrams to the server whenever they have data to send, without any formal connection process.
(By ChatGPT)



