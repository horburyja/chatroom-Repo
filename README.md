# Chatroom

Implementation of chatroom server and client using ports in C.

# Project Details

when a client (irc) wants to talk to a server (ircd), simply execute a client from the terminal with:

./irc localhost

This would connect to a server running on localhost (port 8075). Now a client could read input from the keyboard and send them to the server. When a response comes from the server, it displays the response on its standard output.

The server waits for an inbound request from clients on port 8075 and adds the socket of the client to an array of sockets to talk with when sia d request arrives. The server handles all communications directly through the select API. This means, given n client sockets and a server socket, the server will listen to all of them unitl one socket had data ready to read. When this happens the server can handle that socket.

    If it is the server socket sending data, that means another client is trying to join the chatroom.
    
    If it is a client socket, then the message from that client is read and sent to all other clients. It is important to note that a message in this case consists of a string of text whose last character is a line feed ('\n').

If a client wants to terminate the chatroom, it can send the message "$exit\n". This, in effect, allows a single user to terminate the entire chatroom.