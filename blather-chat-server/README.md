Welcome to my simple chat server called blather! This project was developed as a part of
the CSCI4061  (Intoduction to Operating Systems) course at the University of Minnesota - Twin Cities.

The Server: Some user starts bl_server which manages the chat "room". The server is non-interactive and will likely only print debugging output as it runs.

The Client: Any user who wishes to chat runs bl_client which takes input typed on the keyboard and sends it to the server. The server broadcasts the input to all other clients who can respond by typing their own input.

Unlike standard internet chat services, blather will be restricted to a single Unix machine and to users with permission to read related files. This project can be extended to run on the internet via the use of sockets (not-demoed here).

Like most interesting projects, blather utilizes a combination of many different system tools. Look for the following items to arise.

Multiple communicating processes: clients to servers

Communication through FIFOs

Signal handling for graceful server shutdown

Alarm signals for periodic behavior

Input multiplexing with poll()

Multiple threads in the client to handle typed input versus info from the server
