#include "blather.h"
#include <poll.h>

// Gets a pointer to the client_t struct at the given index. If the
// index is beyond n_clients, the behavior of the function is
// unspecified and may cause a program crash.
client_t *server_get_client(server_t *server, int idx){
    check_fail(idx > server->n_clients, 0, "idx is beyond n_clients\n");
    return &(server->client[idx]);
}

// Initializes and starts the server with the given name. A join fifo
// called "server_name.fifo" should be created. Removes any existing
// file of that name prior to creation. Opens the FIFO and stores its
// file descriptor in join_fd.
//
// ADVANCED: create the log file "server_name.log" and write the
// initial empty who_t contents to its beginning. Ensure that the
// log_fd is position for appending to the end of the file. Create the
// POSIX semaphore "/server_name.sem" and initialize it to 1 to
// control access to the who_t portion of the log.
//
// LOG Messages:
// log_printf("BEGIN: server_start()\n");              // at beginning of function
// log_printf("END: server_start()\n");                // at end of function
void server_start(server_t *server, char *server_name, int perms){

    char server_fifo[MAXNAME];
    log_printf("BEGIN: server_start()\n");
    server->join_ready = 0; //set join_ready
    strcpy(server->server_name, server_name); //set server_name
    strcpy(server_fifo,server_name);
    strcat(server_fifo,".fifo"); //server FIFO
    remove(server_fifo); //removes any existing fifo with the same name

    int sf = mkfifo(server_fifo,perms); //makes a new fifo
    check_fail(sf == -1, 1, "error when creating server fifo\n");
    dbg_printf("server fifo has been created\n");
    server->join_fd = open(server_fifo,O_RDWR); //opens the fifo and stores its file descriptor in join_fd
    check_fail(server->join_fd == -1, 1, "error when opening server fifo\n");
    dbg_printf("join_fd now holds server fifo's file descriptor\n");
    log_printf("END: server_start()\n");

}

// Shut down the server. Close the join FIFO and unlink (remove) it so
// that no further clients can join. Send a BL_SHUTDOWN message to all
// clients and proceed to remove all clients in any order.
//
// ADVANCED: Close the log file. Close the log semaphore and unlink
// it.
//
// LOG Messages:
// log_printf("BEGIN: server_shutdown()\n");           // at beginning of function
// log_printf("END: server_shutdown()\n");             // at end of function
void server_shutdown(server_t *server){
    log_printf("BEGIN: server_shutdown()\n");
    close(server->join_fd);
    dbg_printf("join fifo has been closed\n");
    char *fd = strcat(server->server_name,".fifo");
    remove(fd); //unlink/remove server fifo

    mesg_t mesg = {};
    mesg.kind = BL_SHUTDOWN;
    server_broadcast(server,&mesg); //send a BL_SHUTDOWN message to all clients

    for(int i =0;i<server->n_clients;i++){
        server_remove_client(server,i); //remove all clients
    }
    dbg_printf("all clients have been removed!\n");
    log_printf("END: server_shutdown()\n");

}

// Adds a client to the server according to the parameter join which
// should have fileds such as name filed in.  The client data is
// copied into the client[] array and file descriptors are opened for
// its to-server and to-client FIFOs. Initializes the data_ready field
// for the client to 0. Returns 0 on success and non-zero if the
// server as no space for clients (n_clients == MAXCLIENTS).
//
// LOG Messages:
// log_printf("BEGIN: server_add_client()\n");         // at beginning of function
// log_printf("END: server_add_client()\n");           // at end of function
int server_add_client(server_t *server, join_t *join){
    log_printf("BEGIN: server_add_client()\n");
    if (server->n_clients == MAXCLIENTS){
        dbg_printf("server has no space for clients\n");
        return 1;
    } else{
        client_t client = {}; //new client

        strncpy(client.name,join->name,strlen(join->name)); //copy client name
        strncpy(client.to_client_fname, join->to_client_fname,strlen(join->to_client_fname)); //copy client to-client fd
        strncpy(client.to_server_fname,join->to_server_fname,strlen(join->to_server_fname)); //copy client to-server fd
        client.to_client_fd = open(client.to_client_fname,O_RDWR); //opens fd for to-client
        check_fail(client.to_client_fd == -1, 1, "error when opening to-client fifo\n");
        client.to_server_fd = open(client.to_server_fname,O_RDWR); //opens fd for to-server
        check_fail(client.to_server_fd == -1, 1, "error when opening to-server fifo\n");
        dbg_printf("to-client and to-server fds opened\n");

        client.data_ready = 0; //initialize data ready to 0
        server->client[server->n_clients] = client; //add client to end of server's client array
        server->n_clients++; //increment size of server's client array
        dbg_printf("client %s has been added to server\n",client.name);
        log_printf("END: server_add_client()\n");
        return 0;
    }
}

// Remove the given client likely due to its having departed or
// disconnected. Close fifos associated with the client and remove
// them.  Shift the remaining clients to lower indices of the client[]
// preserving their order in the array; decreases n_clients. Returns 0
// on success, 1 on failure.

int server_remove_client(server_t *server, int idx){
    if(server->n_clients < idx || idx < 0){
        dbg_printf("specified idx not in range\n");
        return 1;
    }
    client_t *client = server_get_client(server,idx); //get client at specified idx
    int cc = close(client->to_client_fd); // close to_client fifo
    check_fail(cc == -1, 1, "error when closing to-client fifo\n");

    int cs = close(client->to_server_fd); //close to_server fifo
    check_fail(cs == -1, 1, "error when closing to-server fifo\n");

    int cr = remove(client->to_server_fname); //remove to_server fifo
    check_fail(cr == -1, 1, "error when removing to-server fifo\n");

    int sr = remove(client->to_client_fname); //remove to_client fifo
    check_fail(sr == -1, 1, "error when removing to-client fifo\n");

    server->n_clients--; //decrease size of n_clients
    dbg_printf("Removing client %s from server %s",server_get_client(server,idx)->name,server->server_name);
    for(int i=idx;i<server->n_clients;i++){
        server->client[i] = server->client[i+1]; //shift remaining clients to lower indices
         dbg_printf("shifted client: %s from index: %d to index: %d\n",server->client[i].name,i+1,i);
    }

    return 0;

}

// Send the given message to all clients connected to the server by
// writing it to the file descriptors associated with them.
//
// ADVANCED: Log the broadcast message unless it is a PING which
// should not be written to the log.
void server_broadcast(server_t *server, mesg_t *mesg){
    dbg_printf("broadcasting message to all clients\n");
    for(int i=0;i<server->n_clients;i++){
        int w = write(server_get_client(server,i)->to_client_fd,mesg,sizeof(mesg_t));
        check_fail(w == -1, 1, "error when writing to clients\n");
    }
}

// Checks all sources of data for the server to determine if any are
// ready for reading. Sets the servers join_ready flag and the
// data_ready flags of each of client if data is ready for them.
// Makes use of the poll() system call to efficiently determine which
// sources are ready.
//
// NOTE: the poll() system call will return -1 if it is interrupted by
// the process receiving a signal. This is expected to initiate server
// shutdown and is handled by returning immediagely from this function.
//
// LOG Messages:
// log_printf("BEGIN: server_check_sources()\n");             // at beginning of function
// log_printf("poll()'ing to check %d input sources\n",...);  // prior to poll() call
// log_printf("poll() completed with return value %d\n",...); // after poll() call
// log_printf("poll() interrupted by a signal\n");            // if poll interrupted by a signal
// log_printf("join_ready = %d\n",...);                       // whether join queue has data
// log_printf("client %d '%s' data_ready = %d\n",...)         // whether client has data ready
// log_printf("END: server_check_sources()\n");               // at end of function
void server_check_sources(server_t *server){
    log_printf("BEGIN: server_check_sources()\n");
    struct pollfd pfds[server->n_clients+1]; //poll struct to handle all clients + a join

    for(int i=0;i<=server->n_clients;i++){
        if (i == 0){ // poll for join
             pfds[i].fd = server->join_fd;
             pfds[i].events = POLLIN;
             pfds[i].revents = 0;
        } else { //poll for clients
            pfds[i].fd = server->client[i-1].to_server_fd;
            pfds[i].events = POLLIN;
            pfds[i].revents = 0;
        }
    }
    log_printf("poll()'ing to check %d input sources\n",server->n_clients+1);

    int ret = poll(pfds, server->n_clients+1, -1); //BLOCKS
    log_printf("poll() completed with return value %d\n",ret);

    if(ret == -1){
        log_printf("poll() interrupted by a signal\n");
    } else {

        if(pfds[0].revents & POLLIN){ //join is ready
            server->join_ready = 1;

        }

        log_printf("join_ready = %d\n",server->join_ready);
        for(int i=1;i<server->n_clients+1;i++){
            if(pfds[i].revents & POLLIN ){ //client has data ready
                server->client[i-1].data_ready = 1;
            }
            log_printf("client %d '%s' data_ready = %d\n",i-1,server->client[i-1].name,server->client[i-1].data_ready);


        }
    }

    log_printf("END: server_check_sources()\n");

}
// Return the join_ready flag from the server which indicates whether
// a call to server_handle_join() is safe.
int server_join_ready(server_t *server){
    return server->join_ready;
}

// Call this function only if server_join_ready() returns true. Read a
// join request and add the new client to the server. After finishing,
// set the servers join_ready flag to 0.
//
// LOG Messages:
// log_printf("BEGIN: server_handle_join()\n");               // at beginnning of function
// log_printf("join request for new client '%s'\n",...);      // reports name of new client
// log_printf("END: server_handle_join()\n");                 // at end of function
void server_handle_join(server_t *server){
    log_printf("BEGIN: server_handle_join()\n");
    join_t join = {}; //new join
    int nread = read(server->join_fd, &join, sizeof(join_t)); //read join_fd to new join
    dbg_printf("read join_fd to new join\n");
    check_fail(nread == -1, 1, "Error when reading from join\n");
    log_printf("join request for new client '%s'\n",join.name);
    int f = server_add_client(server, &join); //add new client to the server
    check_fail(f == 1, 1, "server is full, client not added\n");
    mesg_t joined = {};
    joined.kind = BL_JOINED;
    strncpy(joined.name, join.name,strlen(join.name));
    server->join_ready =0;
    server_broadcast(server,&joined);
    dbg_printf("join message has been broadcasted\n");
    log_printf("END: server_handle_join()\n");
}

// Return the data_ready field of the given client which indicates
// whether the client has data ready to be read from it.
int server_client_ready(server_t *server, int idx){
    return server->client[idx].data_ready;
}

// Process a message from the specified client. This function should
// only be called if server_client_ready() returns true. Read a
// message from to_server_fd and analyze the message kind. Departure
// and Message types should be broadcast to all other clients.  Ping
// responses should only change the last_contact_time below. Behavior
// for other message types is not specified. Clear the client's
// data_ready flag so it has value 0.
//
// ADVANCED: Update the last_contact_time of the client to the current
// server time_sec.
//
// LOG Messages:
// log_printf("BEGIN: server_handle_client()\n");           // at beginning of function
// log_printf("client %d '%s' DEPARTED\n",                  // indicates client departed
// log_printf("client %d '%s' MESSAGE '%s'\n",              // indicates client message
// log_printf("END: server_handle_client()\n");             // at end of function
void server_handle_client(server_t *server, int idx){
    log_printf("BEGIN: server_handle_client()\n");
    mesg_t mesg = {}; //new message
    int nread = read(server_get_client(server, idx)->to_server_fd, &mesg, sizeof(mesg_t)); //read a message from to_server_fd
    check_fail(nread == -1, 1, "error when reading from client's to_server_fd\n");

    if (mesg.kind == BL_DEPARTED){ //departed message
      log_printf("client %d '%s' DEPARTED\n", idx,server_get_client(server,idx)->name);
      server_remove_client(server, idx);
      server_broadcast(server, &mesg);
      dbg_printf("departed message has been broadcasted\n");
    } else if (mesg.kind == BL_MESG) { //standard message
        log_printf("client %d '%s' MESSAGE '%s'\n",idx,server_get_client(server,idx)->name,mesg.body);
        server_broadcast(server, &mesg);
        dbg_printf("standard message has been broadcasted\n");
    }
    server_get_client(server, idx)->data_ready = 0; //reset data ready of client
    log_printf("END: server_handle_client()\n");
}
//void server_tick(server_t *server);
//void server_ping_clients(server_t *server);
////void server_remove_disconnected(server_t *server, int disconnect_secs);
//void server_write_who(server_t *server);
//void server_log_message(server_t *server, mesg_t *mesg);
