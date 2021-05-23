#include "commando.h"

// cmd.c: functions related the cmd_t struct abstracting a
// command. Most functions maninpulate cmd_t structs.

cmd_t *cmd_new(char *argv[]){
    cmd_t *new_cmd = malloc(sizeof(cmd_t)); //allocates space for new_cmd
	for (int i=0; i<ARG_MAX+1; i++ ) {
		new_cmd->argv[i] = NULL; //initailzes all values in argv to null which also ensures that new_cmd->argv[] is null terminated
	}
	int index = 0;
	while (argv[index] != NULL) {
		new_cmd->argv[index] = strdup(argv[index]); //makes string copies of argv values
		index++;
    }
    strncpy(new_cmd->name,argv[0],NAME_MAX+1); //set name field to be argv[0]
    new_cmd->finished = 0; //sets to 0 for not finished
    snprintf(new_cmd->str_status,STATUS_LEN+1,"INIT"); //sets str_status to INIT
    new_cmd->status = -1; //sets to -1 to indicate that its not finished
    new_cmd->output = NULL; //sets output to NULL to indicate no output yet
    new_cmd->output_size = -1; //sets output_size to -1 to indicate no bytes have been read (-1 is not a plausible read)
    new_cmd->out_pipe[PREAD] = -1; //sets read end of pipe to -1 (-1 is not a plausible value)
    new_cmd->out_pipe[PWRITE] = -1; //sets write end of pipe to  -1 (-1 is not a plausible value)
    new_cmd->pid = -1; //pid set to -1 as valid pids are always positive
    return new_cmd;


}
void cmd_free(cmd_t *cmd){

    for(int i =0;i<ARG_MAX+1;i++){
        free(cmd->argv[i]); //deallocates argv array
    }

    free(cmd->output); //if output is set, its freed. Does nothing if its NULL
    free(cmd); //deallocates cmd
}

void cmd_start(cmd_t *cmd){
   pipe(cmd->out_pipe); //creates pipe at out_pipe
   snprintf(cmd->str_status,STATUS_LEN+1,"RUN"); //str_status changed to RUN
   cmd->pid = fork(); //fork a new process and capture it into pid (child pid is set to pid field)
   //child process is 0, else parent process
   if(cmd->pid == 0) {// child
        dup2(cmd->out_pipe[PWRITE],STDOUT_FILENO); //directs/writes standard output to the pipe
        close(cmd->out_pipe[PREAD]); //child closes read end of pipe
        execvp(cmd->name,cmd->argv); //launch a new program with output directed to the pipe
        perror("exec failed"); //should not reach here. If so, error out
        exit(1);
   } else if(cmd->pid == -1){ //error, no child process created
        perror("Fork failed: child process not created");
        exit(1);
   }
    else {//parent
        close(cmd->out_pipe[PWRITE]); //parent closes write end of pipe
   }
}

void cmd_update_state(cmd_t *cmd, int block){

    if(cmd->finished == 0){ //will update state of cmd as its not finished

        int waiting = waitpid(cmd->pid, &cmd->status,block); //waits selectively for process and passes block
        if(waiting > 0){ //child has state change
            if(WIFEXITED(cmd->status)!= 0){ // child process has exited
                cmd->status = WEXITSTATUS(cmd->status); //return code of exit assigned to status field
                snprintf(cmd->str_status,STATUS_LEN+1,"EXIT(%d)",cmd->status); //change str_status field to EXIT(status)
                cmd->finished = 1; //finish field set to 1 for future status updates to ignore completed command
                cmd_fetch_output(cmd);//call to read contents of the pipe into cmd's output buffer
                printf("@!!! %s[#%d]: %s\n",cmd->name,cmd->pid,cmd->str_status); // prints program status

            }
        } else if (waiting == -1){ //error, status of child process not available
            perror("Status of child process not available");
            exit(1); //exits as an error has occured
        }
    }
}
char *read_all(int fd, int *nread){
    int  cur_pos = BUFSIZE; //current size of buffer
    int total = 0; //number of bytes read
	char *buf = malloc(sizeof(char)*BUFSIZE); //initial allocation of bytes

    total = read(fd, buf, BUFSIZE); //initial read of bytes from fd

    while (total== cur_pos) { //limits total of bytes read
		cur_pos *= 2; // increases buffer size by doubling
		char *new_buf = realloc(buf,(sizeof(char)*(cur_pos))); //resizes buffer with new cur_pos
		int diff = (cur_pos) - total; // bytes to be read after accounting for previously read bytes
		total += read(fd, &new_buf[total], diff); //reads bytes from fd into reallocated buffer starting at end position of previous read
		buf = new_buf; //sets buf to reallocated buffer
    }

	*nread = total; //sets nread to number of bytes read
	buf[total] = '\0'; //null terminates character output
	return buf;
}

void cmd_fetch_output(cmd_t *cmd){

    if(cmd->finished == 0){ //cmd is not finished
        printf("%s[#%d] not finished yet\n",cmd->name,cmd->pid); //prints error
    }
    else {
        cmd->output = read_all(cmd->out_pipe[PREAD],&cmd->output_size); //retreives output from out_pipe, updates output_size, and sets output
        close(cmd->out_pipe[PREAD]); //closes read end of pipe
    }
}

void cmd_print_output(cmd_t *cmd){
    if(cmd->output == NULL){ //no output set
        printf("%s[#%d] : output not ready\n",cmd->name,cmd->pid); //prints error message
    } else {
        write(STDOUT_FILENO,cmd->output,cmd->output_size); //writes command output to standard output
    }
}
