#include "blather.h"


simpio_t simpio_actual;
simpio_t *simpio = &simpio_actual;


pthread_t user_thr; //thread managing user
pthread_t server_thr; //thread managing server
join_t join = {}; //join struct


//user thread{
//  repeat:
//    read input using simpio
//    when a line is ready
//    create a mesg_t with the line and write it to the to-server FIFO
//  until end of input
//  print "End of Input, Departing"
//  write a DEPARTED mesg_t into to-server
//  cancel the server thread
//

void *user_thread(void *arg){
    dbg_printf("entering user thread method\n");
    int fd = open(join.to_server_fname,O_WRONLY);
    check_fail(fd == -1, 1, "error when opening to-server fifo\n");
    while(1){
        //reset simpio and display prompt
        simpio_reset(simpio);
        iprintf(simpio,""); //prompt is displayed
        dbg_printf("prompt has been displayed\n");

        while(!simpio->line_ready && !simpio->end_of_input){
                simpio_get_char(simpio); //read input using simpio until line is ready
        }
        dbg_printf("line is ready to be read\n");
        mesg_t mesg = {};

        mesg.kind = BL_MESG; //default message kind set to a standard message
        strcpy(mesg.name,join.name);

        strcpy(mesg.body,simpio->buf); //copy message body to mesg

        if(simpio->end_of_input){
            iprintf(simpio,"End of Input, Departing\n");
            mesg.kind = BL_DEPARTED;                 //end of input reached, update message kind

        }

        int nbytes = write(fd, &mesg, sizeof(mesg_t)); //write message into to-server
        check_fail(nbytes == -1, 1, "error when writing message\n");
        dbg_printf("message has been written to to-server\n");

        if(simpio->end_of_input){
            pthread_cancel(server_thr); // cancel the server thread
            dbg_printf("server thread has been cancelled, exiting.\n");
            break;
        }

    }

    dbg_printf("end of client_thread method\n");
    return NULL;
}
//server thread{
//  repeat:
//    read a mesg_t from to-client FIFO
//    print appropriate response to terminal with simpio
//  until a SHUTDOWN mesg_t is read
//  cancel the user thread

void *server_thread(void *args){
    dbg_printf("entering server_thread method \n");
    //open up to-client FIFO to read from
    int fd = open(join.to_client_fname,O_RDONLY);
    check_fail(fd == -1, 1, "error when opening to-client fifo\n");
    while (1){
        mesg_t mesg = {};
        int nbytes = read(fd, &mesg, sizeof(mesg_t)); //read a message from to-client FIFO
        check_fail(nbytes==-1, 1, "error when reading bytes for message!\n");
        dbg_printf("message successfully read!\n");

        //print appropriate response to terminal with simpio
        if(mesg.kind == BL_MESG){
            dbg_printf("message kind was BL_MESG\n");
            //[Bruce] : check this out
            iprintf(simpio,"[%s] : %s\n",mesg.name,mesg.body);
        } else if(mesg.kind == BL_JOINED){
            dbg_printf("message kind was BL_JOINED\n");
            //	-- Bruce JOINED --
            iprintf(simpio,"-- %s JOINED --\n",mesg.name);
        } else if (mesg.kind == BL_DEPARTED){
            dbg_printf("message kind was BL_DEPARTED\n");
            //	-- Clark DEPARTED --
            iprintf(simpio,"-- %s DEPARTED --\n",mesg.name);

        }  else if(mesg.kind == BL_SHUTDOWN){ //  SHUTDOWN mesg_t has been reached, break from loop
            dbg_printf("message kind was BL_SHUTDOWN\n");
            //	!!! server is shutting down !!!
            iprintf(simpio,"!!! server is shutting down !!!\n");
            break;
        }

    }
    pthread_cancel(user_thr); // cancel the user thread

    dbg_printf("user thread has been cancelled ... exiting server thread method\n");
    return NULL;
}

//read name of server and name of user from command line args
//create to-server and to-client FIFOs
//write a join_t request to the server FIFO
//start a user thread to read inpu
//start a server thread to listen to the server
//wait for threads to return
//restore standard terminal output
//

int main(int argc,char *argv[]){

    dbg_printf("beginning client main method");

    char to_client[MAXNAME]; //to_client fifo name

    char to_server[MAXNAME]; //to_server fifo name

    char server_fifo[MAXNAME]; //server fifo name

    char prompt[MAXNAME];
    snprintf(prompt,MAXNAME, "%s>>", argv[2]); //create a prompt string


    simpio_set_prompt(simpio,prompt); // set the prompt
    simpio_reset(simpio);   // initialize io
    simpio_noncanonical_terminal_mode();  // set the terminal into a compatible mode

    strcpy(server_fifo,argv[1]);
    strcat(server_fifo,".fifo"); //server fifo

	snprintf(to_client,MAXNAME,"%d_to-client.fifo",getpid()); //to_client fifo using pid
	snprintf(to_server,MAXNAME,"%d_to-server.fifo",getpid()); //to_server fifo using pid

    int sf = mkfifo(to_client, DEFAULT_PERMS);
    check_fail(sf == -1, 1, "error when creating to-client fifo\n");
    int cf = mkfifo(to_server, DEFAULT_PERMS);
    check_fail(cf == -1, 1, "error when creating to-server fifo\n");
    dbg_printf("to client and to server fifos created!\n");

    strncpy(join.to_client_fname, to_client,strlen(to_client)); //copy to_client fifo into join
    strncpy(join.to_server_fname, to_server,strlen(to_server)); //copy to_server fifo into join
    strncpy(join.name, argv[2],strlen(argv[2])); //update join name to client name

    int fd = open(server_fifo,O_WRONLY,DEFAULT_PERMS);
    int nbytes = write(fd, &join, sizeof(join_t));
    check_fail(nbytes == -1, 1, "error when writing bytes\n");

    pthread_create(&user_thr, NULL, user_thread, NULL); // start user thread to read input
	pthread_create(&server_thr, NULL, server_thread, NULL); // start server thread to read input

	pthread_join(user_thr, NULL); //wait for user thread
	pthread_join(server_thr, NULL); //wait for server thread
	simpio_reset_terminal_mode();
	printf("\n");  // newline just to make returning to the terminal prettier

    dbg_printf("exiting client main method");
	return 0;

}


