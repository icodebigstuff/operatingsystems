#include "commando.h"
#include <stdio.h>
#include <stdlib.h> //included to use getenv
int main(int argc, char* argv[]){ //length of argv is stored in argc
    setvbuf(stdout, NULL, _IONBF, 0); // Turn off output buffering
    char **tokens = malloc(sizeof(char) * ARG_MAX); //allocates space for tokens
    int *ntok = malloc(sizeof(int)); //allocates space for ntok
    cmdcol_t *cmd_col = malloc(sizeof(cmdcol_t)); //allocates space for cmd_col
    cmd_col->size = 0; //size of cmd_col updated to be zero
    while(1) {
        printf("@> ");
        char arr[MAX_LINE]; //array to store standard input
        if(fgets(arr,MAX_LINE,stdin) == NULL) { // no more input
            printf("\n");
            printf("End of input\n");
            break;

        } else{
            if(argc > 1){ //check to see if input array has more than one value
                if (strncmp(argv[1],"--echo",6) == 0){ // echo input
                    printf("%s", arr);}  //prints standard input
            }
            if (getenv("COMMANDO_ECHO")){ //executes if COMMAND_ECHO has been set
                printf("%s",arr); //prints standard input
            }
            parse_into_tokens(arr,tokens,ntok);
            if (tokens[0]!= NULL){ //check to see if token is not null
               if(strncmp(tokens[0],"help",4) == 0){ //help command
                    printf("COMMANDO COMMANDS\n");
                    printf("help               : show this message\n");
                    printf("exit               : exit the program\n");
                    printf("list               : list all jobs that have been started giving information on each\n");
                    printf("pause nanos secs   : pause for the given number of nanseconds and seconds\n");
                    printf("output-for int     : print the output for given job number\n");
                    printf("output-all         : print output for all jobs\n");
                    printf("wait-for int       : wait until the given job number finishes\n");
                    printf("wait-all           : wait for all jobs to finish\n");
                    printf("command arg1 ...   : non-built-in is run as a job\n");
                } else if (strncmp(tokens[0],"exit",4) == 0){ //exit command
                    break;
                } else if (strncmp(tokens[0],"list",4) == 0){ //list command
                    cmdcol_print(cmd_col); //prints contents of cmds in cmd_col
                }else if(strncmp(tokens[0],"output-all",10) == 0){ //output-all command
                    for(int i =0;i<cmd_col->size;i++){
                    printf("@<<< Output for %s[#%d] (%d bytes):\n",cmd_col->cmd[i]->name,cmd_col->cmd[i]->pid,cmd_col->cmd[i]->output_size); //description of cmd being printed
                    printf("----------------------------------------\n");
                    cmd_print_output(cmd_col->cmd[i]); //prints cmd
                    printf("----------------------------------------\n");
                    }
                } else if (strncmp(tokens[0],"output-for",10) == 0){ //output-for command
                    printf("@<<< Output for %s[#%d] (%d bytes):\n",cmd_col->cmd[atoi(tokens[1])]->name,cmd_col->cmd[atoi(tokens[1])]->pid,cmd_col->cmd[atoi(tokens[1])]->output_size);//prints description of given job number
                    printf("----------------------------------------\n");
                    cmd_print_output(cmd_col->cmd[atoi(tokens[1])]); //prints output at given job
                    printf("----------------------------------------\n");
                } else if (strncmp(tokens[0], "wait-for",8) == 0){ //wait-for command
                    cmd_update_state(cmd_col->cmd[atoi(tokens[1])],DOBLOCK); //wait until given job number finishes
                } else if (strncmp(tokens[0], "wait-all",8) == 0){ //wait-all command
                    cmdcol_update_state(cmd_col,DOBLOCK); //wait for all jobs to finish
                } else if (strncmp(tokens[0], "pause",5) == 0){ //pause command
                    pause_for(atoi(tokens[1]),atoi(tokens[2])); //pause for given nanseconds and seconds
                } else{ //no built in selected, running command as a job
                    cmd_t *new_cmd = cmd_new(tokens); //creates new cmd
                    cmdcol_add(cmd_col,new_cmd); //new cmd added to cmd_col structure
                    cmd_start(cmd_col->cmd[cmd_col->size-1]); //new cmd is started
                }
            }
        cmdcol_update_state(cmd_col,NOBLOCK); //update the state of all child processes
        }
    }
    free(tokens);
    free(ntok);
    cmdcol_freeall(cmd_col);  //free every cmd in col
    free(cmd_col); //free col structure
    return 0;
}
