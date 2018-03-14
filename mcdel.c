// Naz Taylor
// CS 485
// PA 5

#include "../csapp.h"
#include "../mycloud_server.h"
#include "mycloud_library.c"

int main(int argc, char** argv){

	// Initalize variables
	char MachineName[MACHINE_NAME_SIZE];
	char Filename[FILE_NAME_SIZE];
	unsigned int TCPport; 
	unsigned int SecretKey;
	int hostLen;
	int  nameLen;

	// If number of command line srguments are not what was expected print error message
	if(argc != 5){
		fprintf(stderr, "Not vslid number of command line parameters. \n");
		fprintf(stderr, "The format is as follows: %s <MachineName> <TCPport> <SecretKey> <Filename> \n", argv[0] );
		return 0;
	}
	// If number of cammand line arguments are what was expected then continue with program.
	else if (argc == 5){
		hostLen = strlen(argv[1]);

		// Check if host name follows specification.
		if (hostLen > MACHINE_NAME_SIZE)
		{
			fprintf(stderr, "Host name is too long, max length is %d\n",MACHINE_NAME_SIZE);
			return 0;
		}

		// check if the length of the file name follows specification.
		nameLen = strlen(argv[4]);

		if (nameLen > FILE_NAME_SIZE){
			fprintf(stderr, "File name is too long, max length is %d\n",FILE_NAME_SIZE);
			return 0;
		}
		// set variables to match the input of the program
			strncpy(MachineName,argv[1],MACHINE_NAME_SIZE);
			TCPport = atoi(argv[2]);
			SecretKey = atoi(argv[3]);
			strncpy(Filename,argv[4],FILE_NAME_SIZE);
			Filename[nameLen] = '\0';
			nameLen++;
		
	}

	// Call function that actually deletes file from server.
	int result = mycloud_delfile(MachineName, TCPport, SecretKey, Filename);

	// Error handling if function call comes back bad
	if (result < 0)
		printf("Error\n");

	return 0;
}
