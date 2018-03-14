// Naz Taylor
// CS 485
// PA5

#include "../csapp.h"
#include "../mycloud_server.h"
#include "mycloud_library.c"

int main(int argc, char** argv){

	// Initialize variables
	char MachineName[MACHINE_NAME_SIZE];
	char Filename[FILE_NAME_SIZE];
	unsigned int TCPport;
	unsigned int SecretKey;
	int hostLen;
	int nameLen;

	// IF number of command line arguments does not match what was expected then give error message
	if(argc != 5){
		fprintf(stderr, "Not valid number of cammand line arguments. \n");
		fprintf(stderr, "The format is as follows: %s <MachineName> <TCPport> <SecretKey> <Filename> \n", argv[0] );
		return 0;
	}

	// If number of command line arguments is what was expected then continue with program.
	else if (argc == 5){
		hostLen = strlen(argv[1]);

		// Check if host name follows specification.
		if (hostLen > MACHINE_NAME_SIZE)
		{
			fprintf(stderr, "Host name is too long, max length is %d\n",MACHINE_NAME_SIZE);
			return 0;
		}

		nameLen = strlen(argv[4]);

		// Check if file name follows specification
		if (nameLen > FILE_NAME_SIZE){
			fprintf(stderr, "File name is too long, max length is %d\n",FILE_NAME_SIZE);
			return 0;
		}	
			// Set variables to reflect the input into the program
			strncpy(MachineName,argv[1],MACHINE_NAME_SIZE);
			TCPport = atoi(argv[2]);
			SecretKey = atoi(argv[3]);
			strncpy(Filename,argv[4],FILE_NAME_SIZE);
			Filename[nameLen] = '\0';
			nameLen++;
	}
	
	// Clear space and call program to sctuall get the file off of the server.
	unsigned int datalen = 0;
	char * data = (char*)Malloc(MAX_FILE_SIZE);
	int result = mycloud_getfile(MachineName,TCPport,SecretKey,Filename,&data, &datalen);

	// Error handling from function call
	if (result >= 0 && datalen > 0)
	{
		Fwrite(data,datalen,1,stdout);
	}
	else
	{
		fprintf(stderr,"Error\n");
	}
		

	return 0;
}
