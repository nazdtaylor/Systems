// Naz Taylor
// CS 485
// PA 5

#include "../csapp.h"
#include "../mycloud_server.h"

// function to send the key specified and the type of interaction to the server 
void sendKeyAndReqType(rio_t * rio,int clientfd, unsigned int SecretKey, unsigned int requestType){
    
	// declaration of variables
	unsigned int netByte;
   	char* sendData;
   	unsigned int sendLen;

    	//  creation of space to save and send the key and type
    	sendLen = MC_NUM_SIZE*2;
    	sendData = (char*)Malloc(sendLen);
    	// key and type being made sendable
	netByte = htonl(SecretKey);
    	char* pSendData = sendData;
    	memcpy(pSendData,&netByte,MC_NUM_SIZE);
    	pSendData += MC_NUM_SIZE;
    	netByte = htonl(requestType);
    	memcpy(pSendData,&netByte,MC_NUM_SIZE);
    	pSendData +=MC_NUM_SIZE;
	
	// data for key and type in correct format to be used by csapp.c functions
    	Rio_writen(clientfd,sendData,sendLen);
    
}

// Function to put files onto the sercer
int mycloud_putfile(char *MachineName, unsigned int TCPport, unsigned int SecretKey, char *Filename, char *data, unsigned int datalen){
	
	// variable declaration
    	int clientfd;
    	unsigned int netByte,result;    
    	rio_t rio;

    	clientfd = Open_clientfd(MachineName,TCPport);
    	Rio_readinitb(&rio, clientfd);
    
   	char* sendData,*pSendData;
    	unsigned int sendLen;
	
	// call function that sends key and type of interaction to the server
    	sendKeyAndReqType(&rio,clientfd,SecretKey,MC_PUT);
    	
	// clearing space and setting file information of actual file to header variables
	sendLen = FILE_NAME_SIZE + MC_NUM_SIZE;
    	sendData = (char*)Realloc(sendData,sendLen);
    	pSendData = sendData;
    	memcpy(pSendData,Filename,FILE_NAME_SIZE);
    	pSendData += FILE_NAME_SIZE;
    	netByte = htonl(datalen);
    	memcpy(pSendData,&netByte,MC_NUM_SIZE);
    	
	// using csapp.c functions to interact with server 
    	Rio_writen(clientfd, sendData, sendLen);
     	Rio_writen(clientfd, data,datalen);
    
    	char* buf;
    	buf = (char*)Malloc(MC_NUM_SIZE);
    
    	Rio_readnb(&rio,buf,MC_NUM_SIZE );
    	memcpy(&netByte,buf,MC_NUM_SIZE);
    	result = ntohl(netByte);

	// free up space after process is completed
    	Close(clientfd);
    	Free(sendData);
    	Free(buf);

	// return -1 if putting file failed
    	if (result == MC_ERR){
        	return MC_ERR;
    	}
    	// return 0 if putting files succeded
    	return MC_SUCC;   
}

// function to list all files stored on cloud server
int mycloud_listfiles(char *MachineName, unsigned int TCPport, unsigned int SecretKey, char **listbuf, unsigned int *listbuflen){
    	
	// declaration of needed starting variables
	unsigned int result = MC_SUCC;
	int clientfd;
    	unsigned int netByte;    
    	rio_t rio;
    	clientfd = Open_clientfd(MachineName,TCPport);
	
	// determine what information to give to the server and send it
    	Rio_readinitb(&rio, clientfd);
    	sendKeyAndReqType(&rio,clientfd,SecretKey,MC_LIST);
    	// clear space for data being returned from the server
	char* buf;
    	buf = (char*)Malloc(MC_NUM_SIZE);
    	
	// interpreting data from server and setting it to a variable that can be printed as a result
   	Rio_readnb(&rio,buf,MC_NUM_SIZE );
    	memcpy(&netByte,buf,MC_NUM_SIZE);
    	result = ntohl(netByte);

    	if(!result){
        	Rio_readnb(&rio,buf,MC_NUM_SIZE );
        	memcpy(&netByte,buf,MC_NUM_SIZE);
        	*listbuflen = ntohl(netByte);
        	Rio_readnb(&rio,*listbuf,*listbuflen);
    	}
	// free space
    	Close(clientfd);
    	Free(buf);
	
	// show error if listing failes or information sned to server doesnt match
    	if (result == MC_ERR){
        	return MC_ERR;
    	}
	// print files on server
	else{
    		return result;
	}

}

// function to get files off of server and store them client side
int mycloud_getfile(char *MachineName, unsigned int TCPport, unsigned int SecretKey, char *Filename, char **data, unsigned int *datalen){

	// declaration of initially needed variables
    	unsigned int result = MC_SUCC;
	int clientfd;
    	unsigned int netByte;    
    	rio_t rio;
    	clientfd = Open_clientfd(MachineName,TCPport);

	// interpreting and sending key and type to server
    	Rio_readinitb(&rio, clientfd);
    	sendKeyAndReqType(&rio,clientfd,SecretKey,MC_GET);

	// declaration of variables needed to handle information being sent
    	char* sendData;
	char* pSendData;
    	unsigned int sendLen;

	// setting variables being sent
    	sendLen = FILE_NAME_SIZE;
    	sendData = (char*)Malloc(sendLen);
    	pSendData = sendData;
    	memcpy(pSendData,Filename,FILE_NAME_SIZE);
    	Rio_writen(clientfd, sendData, sendLen);

	// creation of a variable with allotted memory for storage of information coming in
    	char* buf;
   	buf = (char*)Malloc(MC_NUM_SIZE);
    
	// getting information using csapp.c functions
    	Rio_readnb(&rio,buf,MC_NUM_SIZE );
    	memcpy(&netByte,buf,MC_NUM_SIZE);
   	result = ntohl(netByte);

	// case if there was nothing to get
    	if(!result){
        	Rio_readnb(&rio,buf,MC_NUM_SIZE );
        	memcpy(&netByte,buf,MC_NUM_SIZE);

        	*datalen = ntohl(netByte);
        	Rio_readnb(&rio,*data,*datalen);
    	}
	// freeing up memory
    	Close(clientfd);
    	Free(sendData);
    	Free(buf);
	
	// print error if something went wrong
    	if (result == MC_ERR){
        	return MC_ERR;
    	}
	// otherwise return requested file
    	return *datalen;
}

// function for deleting files on server
int mycloud_delfile(char *MachineName, unsigned int TCPport, unsigned int SecretKey, char *Filename){
    
	// declaration of initally needed variables
	unsigned int result = MC_SUCC;
    	int clientfd;
    	unsigned int netByte;    
    	rio_t rio;
    	clientfd = Open_clientfd(MachineName,TCPport);

	// getting and sending information about key and type to server
    	Rio_readinitb(&rio, clientfd);
    	sendKeyAndReqType(&rio,clientfd,SecretKey,MC_DEL);

	// variables for data that will be deleted
    	char* sendData;
	char* pSendData;
    	unsigned int sendLen;

	// definition of variables initialized above and sending to server
    	sendLen = FILE_NAME_SIZE;
    	sendData = (char*)Malloc(sendLen);
    	pSendData = sendData;
    	memcpy(pSendData,Filename,FILE_NAME_SIZE);
    	Rio_writen(clientfd, sendData, sendLen);
	
	// clearing space
    	char* buf;
    	buf = (char*)Malloc(MC_NUM_SIZE);
    	
	// recieving from server file should be deleted
    	Rio_readnb(&rio,buf,MC_NUM_SIZE );
    	memcpy(&netByte,buf,MC_NUM_SIZE);
    	result = ntohl(netByte);

	// freeing up of no longer needed memory
    	Close(clientfd);
    	Free(sendData);
    	Free(buf);
	// return error if something went wrong
    	if (result == MC_ERR){
        	return MC_ERR;
    	}
	// give result
    	return result;

}
