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
	int  fileNameLen;
	
	// If command line arguements are not what was expected print error message
	if(argc != 4){
		fprintf(stderr, "Invalid number of command line arguments.\n");
		fprintf(stderr, "The format is as follows: %s <MachineName> <TCPport> <SecretKey> <Filename> \n", argv[0] );
		return 0;
	}
	// If command line arguemnt number is correct then continue with program
	else if (argc == 4){
		hostLen = strlen(argv[1]);

		// CHeck if host name follows specification
		if (hostLen > MACHINE_NAME_SIZE)
		{
			fprintf(stderr, "Host name is too long, max length is %d\n",MACHINE_NAME_SIZE);
			return 0;
		}
		
		// Set variables to be passed into function
		strncpy(MachineName,argv[1],MACHINE_NAME_SIZE);
		TCPport = atoi(argv[2]);
		SecretKey = atoi(argv[3]);
	}

	// Clear space and call function to list files on the server
	unsigned int listbuflen;
	char * listbuf = (char*)Malloc(FILE_NAME_SIZE*1000);
	int result = mycloud_listfiles(MachineName, TCPport, SecretKey, &listbuf,&listbuflen );

/*
#ifdef MC_DEBUG
	fprintf(stderr, "The length of list is %u \n", listbuflen );
#endif
*/	
	char* plistbuf = listbuf;
	
	while(listbuflen){

		Fwrite(plistbuf,FILE_NAME_SIZE,1,stdout);
		printf("\n");

		listbuflen -= FILE_NAME_SIZE;
		plistbuf+= FILE_NAME_SIZE;
	}
	if (result != 0)
		printf("Error\n");
	Free(listbuf);
	return 0;
}
