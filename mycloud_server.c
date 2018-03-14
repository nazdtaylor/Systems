// Naz Taylor
// CS 485
// PA 5

#include <stdio.h>
#include <stdlib.h>
#include "../mycloud_server.h"
#include "../csapp.h"
#include <string.h>

// Global variable initialization
char fileList[MAX_NUM_FILES][FILE_NAME_SIZE];
unsigned int numFiles = 0;

// Functioon to validate key that returns 0 if key is valid and -1 if it is invalid
int validKey(rio_t *rio, unsigned int secretKey) {
    	// initialize variables
	size_t n;
    	char buf[SECRET_KEY_SIZE];
    	unsigned int clientKey, netOrder;

    	// Read binary data into buffer
    	if((n = Rio_readnb(rio, buf, SECRET_KEY_SIZE)) == SECRET_KEY_SIZE) {
        	// Copy binary data from buffer
        	memcpy(&netOrder, &buf, SECRET_KEY_SIZE);
        	clientKey = ntohl(netOrder);        
	
		// Print key
        	printf("Secret Key       = %d\n", clientKey);
        	if(clientKey == secretKey) { return 0; } 
        	else { return -1; }
    }    
    return -1;
}

// Function to determine type of application requested that returns the request type or -1 if invalid request
int getRequest(rio_t *rio) {
    	// initialize variables
	size_t n;
    	char buf[REQUEST_TYPE_SIZE];
    	unsigned int requestType, netOrder;

    	// Read binary data into buffer
    	if((n = Rio_readnb(rio, buf, REQUEST_TYPE_SIZE)) == REQUEST_TYPE_SIZE) {
        	// Copy binary data from buffer
        	memcpy(&netOrder, &buf, REQUEST_TYPE_SIZE);
        	requestType = ntohl(netOrder);

        	return requestType;
    	}
    	return -1;
}

// Function to store request type into buffer and returns 0 if the request is stored and -1 if not
int storeRequest(rio_t *rio, int connfd) {
   	// initialize needed variables
	size_t n;
    	char fileNameBuf[FILE_NAME_SIZE];
    	char fileName[FILE_NAME_SIZE];
    	char fileSizeBuf[MAX_NUM_BYTES_IN_FILE];
    	unsigned int fileSize, netOrder, status, messageSize;
    	char dataBuf[MAX_FILE_SIZE];
    	char *data, *message;
    	FILE *file;   
    	
	// Read file name and copy binary data
    	if((n = Rio_readnb(rio, fileNameBuf, FILE_NAME_SIZE)) == FILE_NAME_SIZE) {
        	memcpy(&fileName, &fileNameBuf, FILE_NAME_SIZE);
        	printf("Filename         = %s\n", fileName);
    	} else {
        	printf("Filename         = NONE\n");
        	status = -1;
    	}

    	// Read file size and copy binary data
    	if((n = Rio_readnb(rio, fileSizeBuf, MAX_NUM_BYTES_IN_FILE)) == MAX_NUM_BYTES_IN_FILE) {
        	memcpy(&netOrder, &fileSizeBuf, MAX_NUM_BYTES_IN_FILE);
        	fileSize = ntohl(netOrder);
    	} else {
        	status = -1;
    	}

    	// Read file data
    	if((n = Rio_readnb(rio, dataBuf, fileSize)) == fileSize) {
        	// Allocate memory for the data
        	data = (char*) malloc (sizeof(char)*fileSize);
        	if(data == NULL) { fprintf(stderr, "Memory Error\n"); return -1; }
        	// Copy binary data from buffer
        	memcpy(data, &dataBuf, fileSize);
    	} else {
        	status = -1;
    	}

    	// Write to file and update file list
    	if((file = Fopen(fileName, "w")) != NULL) {
        	Fwrite(data, sizeof(char), fileSize, file);
        	Fclose(file);
        	if(addFileToList(fileName) == 0) { status = 0; }
        	else {status = -1; }
    	} else {
        	status = -1;
    	}
    	free(data);

   	// The remainder of the function stores the response
    	messageSize = STATUS_SIZE;
    	// Allocate memory
    	message = (char*) malloc (sizeof(char*)*messageSize);
    	if(message == NULL) { fprintf(stderr, "Memory Error\n"); return -1; }
    	char *messagePtr = message;
    	netOrder = htonl(status);
    	memcpy(messagePtr, &netOrder, STATUS_SIZE);
    	messagePtr += STATUS_SIZE;

    	// Send the response message
    	Rio_writen(connfd, message, messageSize);
	// Fre memory
    	free(message);

    	return status;
}

// Function to get requested application that returns 0 if it is retreived and -1 if it doesnt
int retrieveRequest(rio_t *rio, int connfd) {
    	// initailze variables
	size_t n;
    	char fileNameBuf[FILE_NAME_SIZE];
    	char fileName[FILE_NAME_SIZE];
    	unsigned int fileSize, netOrder, status, messageSize;
    	char *data, *message;
    	FILE *file;

        // Read file name and copy binary data
    	if((n = Rio_readnb(rio, fileNameBuf, FILE_NAME_SIZE)) == FILE_NAME_SIZE) {
        	memcpy(&fileName, &fileNameBuf, FILE_NAME_SIZE);
        	printf("Filename         = %s\n", fileName);
    	} else {
        	printf("Filename         = NONE\n");
        	status = -1;
    	}

    	// Get response message
    	// Check if file is in list
    	if(fileInList(fileName) == -1) { fileSize = 0; status = -1; }
    	else {
        	// Check if file exists and open it
        	file = fopen(fileName, "r");
        	if(file == 0) { fprintf(stderr, "Cannot open input file\n"); fileSize = 0; status = -1; }
        	else {
            	fseek(file, 0, SEEK_END);
            	fileSize = ftell(file);
            	rewind(file);
            	// Allocate memory for the data buffer
            	data = (char*) malloc (sizeof(char)*fileSize);
            	if(data == NULL) { fprintf(stderr, "Memory Error\n"); return -1; }
            	// Copy file data into data buffer
            	if((n = fread(data, 1, fileSize, file)) == fileSize) { fclose(file); status = 0; }
            	else { fileSize = 0; status = -1; }
        	}
    	}

	// The remainder of the function puts binary data into a buffer
    	messageSize = STATUS_SIZE + MAX_NUM_BYTES_IN_FILE + fileSize;
    	message = (char*) malloc (sizeof(char*)*messageSize);
    	if(message == NULL) { fprintf(stderr, "Memory Error\n"); return -1; }
    	char *messagePtr = message;
    	netOrder = htonl(status);
    	memcpy(messagePtr, &netOrder, STATUS_SIZE);
    	messagePtr += STATUS_SIZE;
    	netOrder = htonl(fileSize);
    	memcpy(messagePtr, &netOrder, MAX_NUM_BYTES_IN_FILE);
    	messagePtr += MAX_NUM_BYTES_IN_FILE;
    	memcpy(messagePtr, data, fileSize);
    	messagePtr += fileSize;
    	free(data);

    	// Send the response message
    	Rio_writen(connfd, message, messageSize);
    	free(message);

    	return status;
}

// Function to process a request to delete that returns 0 if the delete succeeds and -1 if it fails
int deleteRequest(rio_t *rio, int connfd) {
    	// initalize variables
	size_t n;
    	char fileNameBuf[FILE_NAME_SIZE];
    	char fileName[FILE_NAME_SIZE];
    	unsigned int netOrder, status, messageSize;
    	char *message;   

    	// Read file name and copy data from buffer
    	if((n = Rio_readnb(rio, fileNameBuf, FILE_NAME_SIZE)) == FILE_NAME_SIZE) {
        	memcpy(&fileName, &fileNameBuf, FILE_NAME_SIZE);
        	printf("Filename         = %s\n", fileName);
    	} else {
        	printf("Filename         = NONE\n");
        	status = -1;
    	}

    	// Check if file is in list and remove it
    	if(removeFileFromList(fileName) == -1) { status = -1; }
    	else {
        	if(remove(fileName) != 0) { status = -1; }
        	else { status = 0; }
    	}
    	messageSize = STATUS_SIZE;

    	// Allocate memory for the message buffer and copy status into it
    	message = (char*) malloc (sizeof(char*)*messageSize);
    	if(message == NULL) { fprintf(stderr, "Memory Error\n"); return -1; }
    	char *messagePtr = message;
 
    	netOrder = htonl(status);
    	memcpy(messagePtr, &netOrder, STATUS_SIZE);
    	messagePtr += STATUS_SIZE;

    	// Send the response message
    	Rio_writen(connfd, message, messageSize);
    	free(message);

    	return status;
}

// Function to process list request that returns 0 if the list is displayed and -1 if not
int listFilesRequest(rio_t *rio, int connfd) {
    	// initalize variables
	unsigned int datalen, netOrder, status, messageSize;
    	char *message;
    	datalen = numFiles * FILE_NAME_SIZE;
    	messageSize = STATUS_SIZE + MAX_NUM_BYTES_IN_FILE + datalen;

    	// Allocate memory for the message buffer and copy binary data into it
    	message = (char*) malloc (sizeof(char*)*messageSize);
    	if(message == NULL) { fprintf(stderr, "Memory Error\n"); return -1; }
    	char *messagePtr = message;
    	status = 0;
    	netOrder = htonl(status);
    	memcpy(messagePtr, &netOrder, STATUS_SIZE);
    	messagePtr += STATUS_SIZE;
    	netOrder = htonl(datalen);
    	memcpy(messagePtr, &netOrder, MAX_NUM_BYTES_IN_FILE);
    	messagePtr += MAX_NUM_BYTES_IN_FILE;
    	memcpy(messagePtr, fileList, datalen);
    	messagePtr += datalen;

    	// Send the response message
    	Rio_writen(connfd, message, messageSize);
    	free(message);

    	return status;
}

// Function that adds the file name to the last element of the file list returns 0 if the file is added to the list, -1 if not added
int addFileToList(char *fileName) {
    	if((fileInList(fileName)) == -1 && (numFiles < MAX_NUM_FILES)) {
        	strcpy(fileList[numFiles], fileName);
        	numFiles++;
        	return 0;
    	}
    	return -1;
}

// Function that removes the file name from the file list and makes remaining elements continuous returns 0 if the file is removed from the list and a -1 if it is not removed
int removeFileFromList(char *fileName) {
    	int i;
    	if(((i = fileInList(fileName)) != -1) && (i < MAX_NUM_FILES - 1)) {
        	memmove(fileList[i], fileList[i+1], numFiles * FILE_NAME_SIZE);
        	numFiles--;
        	return 0;
    	} else if(i == MAX_NUM_FILES - 1) {
        	numFiles--;
        	return 0;
    	}
    	return -1;
}

// Function that gets and returns the index of the file if in the list and a -1 if the files is not there
int fileInList(char *fileName) {
    	int i;
    	for(i = 0; i < numFiles; i++) {
        	if(strcmp(fileList[i], fileName) == 0) { return i; }
    	}
    	return -1;
}

// Main function
int main(int argc, char **argv)
{
        // Initialization of variables
        int listenfd, connfd, port;
        socklen_t clientlen;
        struct sockaddr_in clientaddr;
        struct hostent *hp;
        char *haddrp;
        unsigned int secretKey;

        // check if number of command line arguments is corerct
        if (argc != 3) {
                fprintf(stderr, "usage: %s <port> <secret key>\n", argv[0]);
                exit(0);
        }
        // set variables to command line arguments that need to be compared to client applications
        port = atoi(argv[1]);
        secretKey = atoi(argv[2]);

        // check port
        listenfd = Open_listenfd(port);
        while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);

        // Determine the domain name and IP address of the client
        hp = Gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr,
                           sizeof(clientaddr.sin_addr.s_addr), AF_INET);
        haddrp = inet_ntoa(clientaddr.sin_addr);
        printf("server connected to %s (%s)\n", hp->h_name, haddrp);

        // set variables to error in case request fails
        int requestType = -1;
        int status = -1;
        rio_t rio;
        Rio_readinitb(&rio, connfd);

	// The rest of the function is to display information about requests from the server	
        // Make sure key matches
        if(validKey(&rio, secretKey) == 0) {
            requestType = getRequest(&rio);
            if(requestType == 0) {
                printf("Request Type     = get\n");
                status = retrieveRequest(&rio, connfd);
            } else if(requestType == 1) {
                printf("Request Type     = put\n");
                status = storeRequest(&rio, connfd);
            } else if(requestType == 2) {
                printf("Request Type     = del\n");
                status = deleteRequest(&rio, connfd);
            } else if(requestType == 3) {
                printf("Request Type     = list\n");
                status = listFilesRequest(&rio, connfd);
            } else {
                printf("Request Type     = invalid(%d)\n", requestType);
            }
        }

        // print status of requessted operation
        if(status == 0) { printf("Operation Status = success\n"); }
        else { printf("Operation Status = error\n"); }
        printf("------------------------------------\n");
        Close(connfd);
        }
        exit(0);
}

