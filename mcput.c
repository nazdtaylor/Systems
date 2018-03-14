// Naz Taylor
// CS 485
// PA5

#include "../csapp.h"
#include "../mycloud_server.h"
#include "mycloud_library.c"

int main(int argc, char** argv){
	// Initialize variables needed
	char MachineName[MACHINE_NAME_SIZE];
	char Filename[FILE_NAME_SIZE];
	unsigned int TCPport;
	unsigned int SecretKey;
	int hostLen;
	int nameLen;

	// If the number of command line srguments is not what the program expests print an error message
	if(argc != 5){
		fprintf(stderr, "Invalid number of cammand line parameters. \n");
		fprintf(stderr, "The format is as follows: %s <MachineName> <TCPport> <SecretKey> <Filename> \n", argv[0] );
		return 0;
	}
	// If command line arguments are correct then continue with program
	else if (argc == 5){
		hostLen = strlen(argv[1]);
		// check if host name is within specification
		if (hostLen > MACHINE_NAME_SIZE)
		{
			fprintf(stderr, "Host name is too long, max length is %d\n",MACHINE_NAME_SIZE);
			return 0;
		}

		nameLen = strlen(argv[4]);
		// Check if length of file name follows specification.
		if (nameLen > FILE_NAME_SIZE-1){
			fprintf(stderr, "File name is too long, max length is %d\n",FILE_NAME_SIZE);
			return 0;
		}
		else{
			// Give variables value that reflects the input.
			strncpy(MachineName,argv[1],MACHINE_NAME_SIZE);
			TCPport = atoi(argv[2]);
			SecretKey = atoi(argv[3]);
			strncpy(Filename,argv[4],FILE_NAME_SIZE);
			Filename[nameLen] = '\0';
			nameLen++;
		}
	}

	char buf[MAX_FILE_SIZE];

	size_t datalen;

    datalen = Fread(buf,sizeof(char),MAX_FILE_SIZE ,stdin);
#ifdef MC_DEBUG
	fprintf(stderr, "The data length is %u \n",datalen );
#endif
	// call function that puts specified file on the server
	int res = mycloud_putfile(MachineName, TCPport, SecretKey, Filename, buf , (unsigned int)datalen);

	// Error handling if function call comes back bad
	if (res < 0){
		printf("Error\n");
	}
	return 0;
}
