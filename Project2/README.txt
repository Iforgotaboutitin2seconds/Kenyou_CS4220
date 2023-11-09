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
Enter file name: m3.txt

You should be able to see the results.

----------------------------------------------------------------------------

This is how Kenyou Teoh Apprach the problems:

Question 1: How do I make a UDP server?

After some research I figure that TCP and UDP is simllar and different. 
That:
In terms of server and client architecture, a TCP server must accept connections from clients and often communicates with them over long-lived connections. A UDP server, on the other hand, listens for incoming datagrams from any number of clients and often processes each one independently, without needing to maintain a continuous connection. Clients using TCP need to establish a connection before sending data to the server, while UDP clients can just send datagrams to the server whenever they have data to send, without any formal connection process.
(By ChatGPT)

That means I cannot reuse code from TCP. That sucks.

With this link https://nikhilroxtomar.medium.com/udp-client-server-implementation-in-c-idiot-developer-c61ce6369352
I am able to get a basic idea of how UDP server.c and client.c should looks like.
After some testing I figure it would be better just set some value as constant, looks better.

Instead of taking the filename as argument, I make it as user input. Because I can.
I also make PORT as constant. I just see it as annoying.

That about sum up how UDP server.c and client.c is made.
After some testing it is working.

Question 2: How do I implement the Go Back-N protocol

I take a look in https://www.baeldung.com/cs/networking-go-back-n-protocol
I figure that UDP is a little bit work for the Go Back-N protocol.
Since UDP just keep sending the file, with Go Back-N protocol it make sure the file it send is acknowledged and tells the client that it works.

I decided to ignore the timer part for now, as it is too much at the moment.
Anyways, after messing around a bit. (ask ChatGPT tons of questions and google/bing it to comfrim it is true)
I decided to make packet as a struct with an integar that is the number of it and the data of it.

ChatGPT tells me an interesting commend. pread, it reads up a number of bytes from a file. Exactally what I needed to break file into packets to send.

Then I ran into this issue
After the first message send from the client, the second message from another client will always be out of order.
Took me a while to realize that is because server sees it as 1 client sending files. After some looking, I did not see in the requirements that we either should or should not deal with. So I leave it be.

Question 3: Time to add Timer.

#include <sys/time.h>
That is what I needed.

I need to use it to keep in track of time, then I make a function to check if it timeout.
Once it is timeout, program exited with 1.

I need to add #define _GNU_SOURCE for enable GNU extensions. So it can actually run the functions.
Annnd it is done. Awesome.

-------------------------------------------------------------------------------
