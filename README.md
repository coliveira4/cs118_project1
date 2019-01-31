# CS118 Project 1

Christina Oliveira
UID: 204803448

##High Level Design Overview

The project contains two parts: A cient and a server that accepts multiple connections (up to 10) from different clients.

###Client:
A socket in non-blocking mode is created and then attempts to connect to the server. We then connect the socket and if the errno is EINPROGRESS it will proceed and the timeout time is checked. One connected, a loop reads files into buffer (1024 byte). This isthen sentover to the server. select() is called before the send() function. Then, if there is no data can be read, the client terminates, as expected by the spec.

###Server:
Threading is used to handle the multiple files. All the connections are first listened to (up to 10). We then get the client socket and the server sends a thread to the connection handler for each connection. The connection handler uses select() to track the timeout of client socket. The connection handler then creates the file and write the contents to it.

##Problems and Solutions

Biggest challenge was ordering the function calls correctly so that I could correctly execute the non-blocking and keeping track of timeout. The TA ended up helping us out at discussion with this.

##References
I referenced my CS111 Project 1 for Socket protocols.
I referenced the sample code in the spec.




