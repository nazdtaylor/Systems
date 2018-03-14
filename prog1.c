#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Function to split up input
// The splitIn function is  a modified version of some code from a former coworker of mine named Bob Grider. I have used this in the past and am using it now with his permission but still felt the need to credit him for his contribution.    
char **splitIn(const char* str, const char* delim, size_t* numtokens)
{
	// Variebles being initilied for the body of the funcion.
	// This first definition is used to copy the original string so it doesnt get overwritten.
	char *s = strdup(str);
	// The next thre variaables are for implementing a dynmaic array. I believe the method used here is fairly common practice to do this type of thing.
	size_t tokens_alloc = 1;
	size_t tokens_used = 0;
	char **tokens = calloc(tokens_alloc, sizeof(char*));
	char *token, *strtok_ctx;
	for (token = strtok_r(s, delim, &strtok_ctx);
		token != NULL;
		token = strtok_r(NULL, delim, &strtok_ctx))
		{
			// See if more space is needed for the tokens and allocate it if necssary
			if (tokens_used == tokens_alloc)
			{
				tokens_alloc *= 2;
				tokens = realloc(tokens, tokens_alloc * sizeof(char*));
			}
			tokens[tokens_used++] = strdup(token);
		}

	// The remainder of the function if for cleaning the allocation process up.
	if (tokens_used == 0)
	{
		free(tokens);
		tokens = NULL;
	}
	else
	{
		tokens = realloc(tokens, tokens_used * sizeof(char*));
	}
	*numtokens = tokens_used;
	free(s);
	return tokens;
}

int main(int argc, char **argv)
{
	// This block is to read in the data file. It was provided by Dr. Finkel and is not my original code except for the error checks.
	unsigned int *data;
	int size;
	FILE *myFile = fopen(argv[1], "r");
	if (myFile == NULL)
	{
		printf("Error opening file\n");
		return 1;
	}
	fread((void *) &size, 4, 1, myFile);
	data = (unsigned int *) malloc(size*4);
	if (data == NULL)
	{
		printf("Failed to allocate memory\n");
		return 1;
	}
	fread(data, 4, size, myFile);
	
	// The following code is for taking an input and parsing it.
	char *line = NULL;
	size_t linelen;
	char **tokens;
	size_t numtokens;
	// Take standard input here.
	while (getline(&line, &linelen, stdin) != -1)
	{
		// Call splitIn funtion to get input split by whitespace.
		tokens = splitIn(line, ", \t\n", &numtokens);
	
		// If too many parameters are input then give error message and stop.	
		if (numtokens > 3)
		{
			printf("Not a valid input\n");
			return 1;
		}

		// If input is "pd" print decimal value.
		else if (strcmp(tokens[0],"pd")==0)
		{
			int element = atoi(tokens[1]);
			printf("%u\n", ((unsigned int *)data)[element]);
		}

		// If input is "px" print in hexadecimal.
		else if (strcmp(tokens[0],"px")==0)
		{
			int element = atoi(tokens[1]);
			char hexNum[9];
			sprintf(hexNum, "%08x", ((unsigned int *)data)[element]);
			printf("%s\n", hexNum);
		}

		// If input is "po" convert to octal and print.
		else if (strcmp(tokens[0],"po")==0)
		{
			int element = atoi(tokens[1]);
			char ocNum[9];
			sprintf(ocNum, "%o", ((unsigned int *)data)[element]);;
			printf("%s\n", ocNum);
		}

		// If input is "b0" print first byte of specified element.
		else if (strcmp(tokens[0],"b0")==0)
		{
			int element = atoi(tokens[1]);
			char hexNum[9];
			sprintf(hexNum, "%08x", ((unsigned int *)data)[element]);
			printf("%.2s\n", hexNum + strlen(hexNum) - 2);
		}

		// If input is "b1" print second byte.
		else if (strcmp(tokens[0],"b1")==0)
		{
			int element = atoi(tokens[1]);
			char hexNum[9];
			sprintf(hexNum, "%08x", ((unsigned int *)data)[element]);
			printf("%.2s\n", hexNum + strlen(hexNum) - 4);
		}

		// If input is "b2" print third byte.
		else if (strcmp(tokens[0],"b2")==0)
		{
			int element = atoi(tokens[1]);
			char hexNum[9];
			sprintf(hexNum, "%08x", ((unsigned int *)data)[element]);
			printf("%.2s\n", hexNum + strlen(hexNum) - 6);
		}

		// If input is "b3" print fourth byte.
		else if (strcmp(tokens[0],"b3")==0)
		{
			int element = atoi(tokens[1]);
			char hexNum[9];
			sprintf(hexNum, "%08x", ((unsigned int *)data)[element]);
			printf("%.2s\n", hexNum);
		}

		// If input is "+" add two specified elements and print in hex.
		else if (strcmp(tokens[0],"+")==0)
		{
			int firstElement = atoi(tokens[1]);
			int secondElement = atoi(tokens[2]);
			unsigned int numSum = ((unsigned int *)data)[firstElement] + ((unsigned int *)data)[secondElement];

			char hexNum[9];
			sprintf(hexNum, "%08x", numSum);
			printf("%s\n", hexNum);
		}

		// If input is "&" and and print specified elements and print in hex.
		else if (strcmp(tokens[0],"&")==0)
		{
			int firstElement = atoi(tokens[1]);
			int secondElement = atoi(tokens[2]);
			unsigned int numAnd = ((unsigned int *)data)[firstElement] & ((unsigned int *)data)[secondElement];

			char hexNum[9];
			sprintf(hexNum, "%08x", numAnd);
			printf("%s\n", hexNum);
		}

		// Print in hex the bitwise or of two elements
		else if (strcmp(tokens[0],"|")==0)
		{
			int firstElement = atoi(tokens[1]);
			int secondElement = atoi(tokens[2]);
			unsigned int numOr = ((unsigned int *)data)[firstElement] | ((unsigned int *)data)[secondElement];

			char hexNum[9];
			sprintf(hexNum, "%08x", numOr);
			printf("%s\n", hexNum);
		}

		// Print exclusive or of two elements in hex.
		else if (strcmp(tokens[0],"^")==0)
		{
			int firstElement = atoi(tokens[1]);
			int secondElement = atoi(tokens[2]);
			unsigned int numXor = ((unsigned int *)data)[firstElement] ^ ((unsigned int *)data)[secondElement];

			char hexNum[9];
			sprintf(hexNum, "%08x", numXor);
			printf("%s\n", hexNum);
		}

		// Left shift element by specified number of bytes.
		else if (strcmp(tokens[0],"<")==0)
		{
			int element = atoi(tokens[1]);
			int shiftNum = atoi(tokens[2]);
			unsigned int shifted = ((unsigned int *)data)[element] << shiftNum;
			char hexNum[9];
			sprintf(hexNum, "%08x", shifted);
			printf("%s\n", hexNum);
		}

		// Right shift element by specified number of bytes.
		else if (strcmp(tokens[0],">")==0)
		{
			int element = atoi(tokens[1]);
			int shiftNum = atoi(tokens[2]);
			unsigned int shifted = ((unsigned int *)data)[element] >> shiftNum;
			char hexNum[9];
			sprintf(hexNum, "%08x", shifted);
			printf("%s\n", hexNum);
		}

		// If input is "q" then quit.
		else if (strcmp(tokens[0],"q")==0)
		{
			return 0;
		}

		// If input is anything other than the ones above give error message and stop.
		else
		{
			printf("Invalid input\n");
			return 1;
		}
		
		// Free space up if tokens werent used
		if (tokens != NULL)
		{
			free(tokens);
		}
	}
	
	// Cleanup lines.
	if (line != NULL)
	{ 
		free(line);
	}	

	fclose(myFile);
	return 0;
}
