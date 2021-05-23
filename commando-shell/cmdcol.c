#include "commando.h"

// cmdcol.c: functions related to cmdcol_t collections of commands.

void cmdcol_add(cmdcol_t *col, cmd_t *cmd){
    if(col->size < MAX_CMDS){ //within size limit of MAX_CMDS
        col->cmd[col->size] = cmd; //adds cmd to col structure
        col->size++; //size of col is updated
    } else { //adding would cause size to exceed MAX_CMDS
        perror("Limit Reached: can not add CMD"); //error message is printed using perror
    }
}

void cmdcol_print(cmdcol_t *col){
    printf("JOB  #PID     STAT   STR_STAT OUTB COMMAND\n"); //column descriptions printed
    for(int i = 0;i<col->size;i++){
        cmd_t *temp = col->cmd[i]; //cmd
        int index = 0;
        char vals[ARG_MAX+1]; //char array to copy copy temp argv array to
        for(int j = 0;j<ARG_MAX+1;j++){
            vals[j] = '\0'; //sets entire array to \0 which is the equivalent of null
        }
        while(temp->argv[index]!= NULL){ //loop to concatante argv commands to vals
            strncat(vals,temp->argv[index],ARG_MAX+1); //concatanates argv[index] to vals
            strncat(vals, " ",2); //adds space to seperate commands
            index++;
    }
    printf("%-1d    #%-1d         %d       %s    %d %-1s\n",i,temp->pid,temp->status,temp->str_status,temp->output_size,vals); //print statement for cmd fields
    }
}

void cmdcol_update_state(cmdcol_t *col, int block) {
    for(int i =0;i<col->size;i++){
        cmd_t *temp = col->cmd[i];
        cmd_update_state(temp,block); //updates each cmd by passing it into cmd_update_state along with the provided block
    }
}

void cmdcol_freeall(cmdcol_t *col){
    for(int i =0;i<col->size;i++){
        cmd_free(col->cmd[i]); //frees each cmd in col
    }
}
