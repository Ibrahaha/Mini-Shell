#ifndef _MAIN_H_
#define _MAIN_H_

	typedef struct sockaddr_in sockaddr_in;
	typedef struct hostent hostent;
	typedef struct in_addr in_addr;
	typedef struct sockaddr sockaddr;

	typedef int (*pfCommande)(int, char**);
	int call(char *name, int argc, char *argv[]);
	pfCommande getCmd(char *name);
	int commandeExiste(char *name);
	int cd(int argc, char *argv[]);
	int whoami(int argc, char *argv[]);
	int pbconnect(int argc, char *argv[]);
	int pbdisconnect(int argc, char *argv[]);
	void disconnect(int signal);
	int stockConnect(char stock, int cSock);
	int launchCmd(int nbArguments, char *arguments[], char *exeName);
	int gestionPipe(int nbArguments, char *arguments[], char *exeName, int nbPipe, int nbCmdPipe, int positionPipe[], int typePipe[], int pipes[]);
	int commandesDisponibles(int argc, char *argv[]);

#endif