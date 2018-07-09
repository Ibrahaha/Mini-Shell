#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "pwd.h"

#ifdef EXE

int main(int argc, char *argv[]){

	int statusCode;
	statusCode = pwd(argc, argv);
	return statusCode;
}

#endif

int pwd(int argc, char *argv[]){

	char currentDir[256];
	if (getcwd(currentDir, sizeof(currentDir)) != NULL){

		write(STDOUT_FILENO, currentDir, strlen(currentDir));
		write(STDOUT_FILENO, "\n", strlen("\n"));
	}
   	else{

   		char *error = "ERREUR : GETCWD()\n";
   		write(STDERR_FILENO, error, strlen(error));   	
   	}
	
	return 0;
}