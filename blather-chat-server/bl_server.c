#include "blather.h"

//REPEAT:
//  check all sources
//  handle a join request if one is ready
//  for each client{
//    if the client is ready handle data from it
//  }
//}


int KEEPGOING = 1; //global variable to update

void handleit(int signum){
    KEEPGOING = 0; //set to 0, breaks from loop
    dbg_printf("server has been signalled\n");
}

int main(int argc, char *argv[]){
    dbg_printf("entering bl_server main method\n");
    if (argc < 2){
        dbg_printf("too little arguments, exiting\n");
        return 1;
    }

    server_t server = {}; //new server
    struct sigaction my_sa = {}; //new sigaction
    my_sa.sa_handler = handleit; //handler setting
    my_sa.sa_flags = SA_RESTART; //restart

    sigaction(SIGINT, &my_sa, NULL); //handle SIGINT
    sigaction(SIGTERM, &my_sa, NULL); //handle SIGTERM
    strncpy(server.server_name,argv[1],strlen(argv[1])); //set server name
    server_start(&server,argv[1],DEFAULT_PERMS); //start server
    while(KEEPGOING){ //while not signalled by SIGINT or SIGTERM
        server_check_sources(&server); //check all sources
        if (server_join_ready(&server)){ // 1 if join ready for server
            server_handle_join(&server); //handles join
        }
        for(int i =0;i<server.n_clients;i++){ //check all clients
            if(server_client_ready(&server,i) == 1){ //1 if client ready
                server_handle_client(&server,i); //handles client
            }
        }
    }
    server_shutdown(&server); //shuts down server
    dbg_printf("exiting bl_server main method\n");
    return 0;


}
