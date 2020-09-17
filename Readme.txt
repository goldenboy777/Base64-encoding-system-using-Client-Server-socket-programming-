Command for compiling
---------------------
gcc server.c -o server
gcc client.c -o client


Command for running Server and Client 
-------------------------------------
Server: <executable code><Server Port number>
Client: <executable code><Server IP Address><Server Port number>


Example:
Server: ./server 3000
Client: ./client localhost 3000


------
Working
-------
Initially, server will be waiting for a TCP connection from the client. Then, client will connect to the server using
server’s TCP port already known to the client. After successful connection, the client accepts the text input from
the user and encodes the input using Base64 encoding system. Once encoded message is computed the client
sends the Message (Type 1 message) to the server via TCP port. After receiving the Message, server should
print the received and original message by decoding the received message, and sends an ACK (Type 2 message)
to the client. The client and server should remain in a loop to communicate any number of messages. Once the
client wants to close the communication, it should send a Message (Type 3 Message) to the server and the TCP
connection on both the server and client should be closed gracefully by releasing the socket resource.

The messages used to communicate contain the following fields:
1. Message_type: integer
2. Message: Character [MSG_LEN], where MSG_LEN(256) is an integer constant
3. <Message> content of the message in Type 3 message can be anything.

Note:Server is able to support multiple clients.

Protocols to follow while Sending different types of message:
------------------------------------------------------------

1)Message format of type 1 should be as follows:
 Type-1: 1<Message_content>   (No space between 1 and message content, any space added will be considered a part of message).
 Type-3: 3<Enter or anything>             (Will disconnect the client with the server).
 Type-2: Acknowledgement message sent by server on successful delivery of the Type-1 message.


Base64 Encoding System Description:
-----------------------------------
Base64 encoding is used for sending a binary message over the net. In this scheme, groups of 24bit are broken
into four 6 bit groups and each group is encoded with an ASCII character. For binary values 0 to 25 ASCII
character ‘A’ to ‘Z’ are used followed by lower case letters and the digits for binary values 26 to 51 & 52 to 61
respectively. Character ‘+’ and ‘/’ are used for binary value 62 & 63 respectively. In case the last group contains
only 8 & 16 bits, then “==” & “=” sequence are appended to the end.



