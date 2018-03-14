// Naz Taylor
// CS 4805
// Program Assignment 4

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <termios.h>
#include "PA4header.h"

void init(){
		// See if we are running interactively
        GBSH_PID = getpid();
        // The shell is interactive if STDIN is the terminal  
        GBSH_IS_INTERACTIVE = isatty(STDIN_FILENO);  

		if (GBSH_IS_INTERACTIVE) {
			// Loop until we are in the foreground
			while (tcgetpgrp(STDIN_FILENO) != (GBSH_PGID = getpgrp()))
					kill(GBSH_PID, SIGTTIN);             
	              
	              
	        // Set the signal handlers for SIGCHILD and SIGINT
			act_child.sa_handler = signalHandler_child;
			act_int.sa_handler = signalHandler_int;			
			
			sigaction(SIGCHLD, &act_child, 0);
			sigaction(SIGINT, &act_int, 0);
			
			// Put ourselves in our own process group
			setpgid(GBSH_PID, GBSH_PID); // we make the shell process the new process group leader
			GBSH_PGID = getpgrp();
			if (GBSH_PID != GBSH_PGID) {
					printf("Error, the shell is not process group leader");
					exit(EXIT_FAILURE);
			}
			// Grab control of the terminal
			tcsetpgrp(STDIN_FILENO, GBSH_PGID);  
			
			// Save default terminal attributes for shell
			tcgetattr(STDIN_FILENO, &GBSH_TMODES);

			// Get the current directory that will be used in different methods
			currentDirectory = (char*) calloc(1024, sizeof(char));
        } else {
                printf("Could not make the shell interactive.\n");
                exit(EXIT_FAILURE);
        }
}

// signal handler for SIGCHLD
void signalHandler_child(int p){
	/* Wait for all dead processes.
	 * We use a non-blocking call (WNOHANG) to be sure this signal handler will not
	 * block if a child was cleaned up in another part of the program. */
	while (waitpid(-1, NULL, WNOHANG) > 0) {
	}
	printf("\n");
}

// Signal handler for SIGINT
void signalHandler_int(int p){
	// We send a SIGTERM signal to the child process
	if (kill(pid,SIGTERM) == 0){
		printf("\nProcess %d received a SIGINT signal\n",pid);
		no_reprint_prmpt = 1;			
	}else{
		printf("\n");
	}
}

// Displays the prompt for the shell
char prompt[1024] = "sish";
void shellPrompt(){
        char hostn[1204] = "";
        gethostname(hostn, sizeof(hostn));
        printf("%s> ", prompt);
}
    
//  Method to change directory
int changeDirectory(char* args[]){
	// If we write no path (only 'cd'), then go to the home directory
	if (args[1] == NULL) {
		chdir(getenv("HOME")); 
		return 1;
	}
	// Else we change the directory to the one specified by the 
	// argument, if possible
	else{ 
		if (chdir(args[1]) == -1) {
			printf(" %s: no such directory\n", args[1]);
            return -1;
		}
	}
	return 0;
}

// Method used to manage the environment variables with different options
int manageEnviron(char * args[], int option){
	char **env_aux;
	switch(option){
		// Case 'environ': we print the environment variables along with
		// their values
		case 0: 
			for(env_aux = environ; *env_aux != 0; env_aux ++){
				printf("%s\n", *env_aux);
			}
			break;
		// Case 'set': we set an environment variable to a value
		case 1: 
			if((args[1] == NULL) && args[2] == NULL){
				printf("%s","Not enought input arguments\n");
				return -1;
			}
			
			// We use different output for new and overwritten variables
			if(getenv(args[1]) != NULL){
				printf("%s", "The variable has been overwritten\n");
			}else{
				printf("%s", "The variable has been created\n");
			}
			
			// If we specify no value for the variable, we set it to ""
			if (args[2] == NULL){
				setenv(args[1], "", 1);
			// We set the variable to the given value
			}else{
				setenv(args[1], args[2], 1);
			}
			break;
	}
	return 0;
}

// handle execution and set the result to specified environment variable
void assignTo(char **args){
   
	FILE *pipein_fp;
	char readbuf[80];
 
	if (( pipein_fp = popen(args[2], "r")) == NULL)
	{
        	perror("popen");
        	exit(1);
	}

	while(fgets(readbuf, 80, pipein_fp)){
        	printf(readbuf, "%c");
 	}

	pclose(pipein_fp);
	setenv(args[1], readbuf, 1);

}


// List proccesses
void listProcs(struct procs_t *procs){
	int i;
	
	for (i = 0; i < MAXJOBS; i++) {
	if (procs[i].pid != 0) {
		printf("[%d] (%d) ", procs[i].jid, procs[i].pid);
		switch (procs[i].state) {
		case BG: 
			printf("Running ");
			break;
		case FG: 
			printf("Foreground ");
			break;
		case ST: 
			printf("Stopped ");
			break;
		default:
			printf("listprocs: Internal error: job[%d].state=%d ", 
			   i, procs[i].state);
		}
		printf("%s", procs[i].cmdline);
	}
	}
}

// Method for launching a program. It can be run in the background or in the foreground
void launchProg(char **args, int background){	 
	 int err = -1;
	 
	 if((pid=fork())==-1){
		 printf("Child process could not be created\n");
		 return;
	 }
	 // pid == 0 implies the following code is related to the child process
	if(pid==0){
		// We set the child to ignore SIGINT signals (we want the parent
		// process to handle them with signalHandler_int)	
		signal(SIGINT, SIG_IGN);
		
		// We set parent=<pathname>/simple-c-shell as an environment variable
		// for the child
		setenv("parent",getcwd(currentDirectory, 1024),1);	
		
		// If we launch non-existing commands we end the process
		if (execvp(args[1],args+1)==err){
			printf("Command not found");
			kill(getpid(),SIGTERM);
		}
	 }
	 
	 // If the process is not requested to be in background, we wait for
	 // the child to finish.
	 if (background == 0){
		 waitpid(pid,NULL,0);
	 }else{
		 printf("Process created with PID: %d\n",pid);
	 }	 
}
 
//Method used to handle the commands entered via the standard input
int commandHandler(char * args[]){
	//int i = 0;
	int j = 0;
	
	//int fileDescriptor;
	//int standardOut;
	
	//int aux;
	int background = 0;
	

	char *args_aux[256];
	
	// We look for the special characters and separate the command itself
	// in a new array for the arguments
	while ( args[j] != NULL){
		if ( (strcmp(args[j],"&") == 0)){
			background = 1;
			break;
		} 
		printf("%s\n", args[j]);
		// Handle replacement of variables if they are mentioned
		size_t len = strlen(args[j]);
		size_t i;
		char *ps;
		for(i = 0; i < len; i++){
			if(args[j][0] == '$'){
				//printf("$\n");
				args[j]++;
				//printf("%s\n", args[j]);
				ps = getenv(args[j]);
				if(! ps){
					printf("'%s' is not set.\n", args[j]);
				}  
				break;
			}
		}
		args_aux[j] = args[j];
		j++;
	}
	printf("\n");
	// 'exit' command quits the shell
	if(strcmp(args[0],"done") == 0) exit(0);
 	// 'comment' command to treat input as comment
	else if (strcmp(args[0],"%") == 0 || args[0][0] == '%') printf("Input treated as comment\n");
	// 'defprompt' command to change prompt displayed
	else if (strcmp(args[0],"defprompt") == 0) strcpy(prompt,args[1]);
	// 'cd' command to change directory
	else if (strcmp(args[0],"cd") == 0) changeDirectory(args);
	// 'listprocs' command
	else if (strcmp(args[0],"listprocs") == 0) listProcs(procs);
	else if (strcmp(args[0],"assignto") == 0) assignTo(args_aux);
	// 'listenv' command to list the environment variables
	else if (strcmp(args[0],"listenv") == 0){
			manageEnviron(args,0);
		}	
	// 'run' command runs specified program
	else if (strcmp(args[0],"run") == 0) launchProg(args_aux,background);
	
	// 'set' command to set environment variables
	else if (strcmp(args[0],"set") == 0) manageEnviron(args,1);
	else{
		printf("Not a valid input.\n");
	}
return 1;
}

//Main method of our shell 
int main(int argc, char *argv[], char ** envp) {
	char line[MAXLINE]; // buffer for the user input
	char * tokens[LIMIT]; // array for the different tokens in the command
	int numTokens;
		
	no_reprint_prmpt = 0; 	// to prevent the printing of the shell
							// after certain methods
	pid = -10; // we initialize pid to an pid that is not possible
	
	// We call the method of initialization
	init();
    // We set our extern char** environ to the environment, so that
    // we can treat it later in other methods
	environ = envp;
	
	// We set shell=<pathname>/simple-c-shell as an environment variable for
	// the child
	setenv("shell",getcwd(currentDirectory, 1024),1);
	
	// Main loop, where the user input will be read and the prompt
	// will be printed
	while(TRUE){
		// We print the shell prompt if necessary
		if (no_reprint_prmpt == 0) shellPrompt();
		no_reprint_prmpt = 0;
		
		// We empty the line buffer
		memset ( line, '\0', MAXLINE );

		// We wait for user input
		fgets(line, MAXLINE, stdin);
	
		// If nothing is written, the loop is executed again
		if((tokens[0] = strtok(line," \n\t")) == NULL) continue;
		
		// We read all the tokens of the input and pass it to our
		// commandHandler as the argument
		numTokens = 1;
		while((tokens[numTokens] = strtok(NULL, " \n\t")) != NULL) numTokens++;
		
		commandHandler(tokens);
		
	}          

	exit(0);
}
