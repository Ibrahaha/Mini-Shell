#define _POSIX_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/wait.h>
#include <string.h>
#include <dlfcn.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include <pwd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "main.h"
#include "ls.h"
#include "cp.h"
#include "mkdir.h"
#include "pwd.h"
#include "cat.h"
#include "du.h"
#include "mv.h"
#include "rm.h"
#include "chmod.h"

#define INCLUS 1
#define EXE 2
#define LIB 3
#define NBCOMMANDE 10


int main(int argc, char *argv[]){
	printf("\033c");
	printf("	##################################################\n");
	printf("	#                                                #\n");
	printf("	#          - Interpréteur de commande -          #\n");
	printf("	#                                                #\n");
	printf("	#                     Polybash                   #\n");
	printf("	#                                                #\n");
	if(TYPECOMPILATION == INCLUS){
	printf("	#              Compilation : Inclus              #\n");
	}
	else if(TYPECOMPILATION == EXE){
	printf("	#                Compilation : Exe               #\n");
	}
	else if(TYPECOMPILATION == LIB){
	printf("	#                Compilation : Lib               #\n");
	}
	printf("	#                                                #\n");
	printf("	##################################################\n");

	signal(SIGINT,disconnect); //Redirection du CTRL + C pour indiquer au serveur qu'on se déconnecte

	char commande[256];
	char currentDir[256];

	int isListened = 0;
	int servfork = -1;
	int servfd;
	socklen_t csize;
 	int servsock = socket(AF_INET, SOCK_STREAM, 0);//On créé le socket serveur
	if(servsock == -1){
		perror("Erreur : socket invalide");
	}

	sockaddr_in addin = { 0 };//On initialise l'addresse du serveur
	addin.sin_addr.s_addr = htonl(INADDR_ANY);//On indique que l'on accepte toute addresse IP à se connecter
	addin.sin_family = AF_INET;//On indique le protocole d'addresse(IPv4 ici)
	int port = 9999;
	do{
		port++;
		addin.sin_port = htons(port);
	}while((bind (servsock, (sockaddr *) &addin, sizeof(addin))<0));//On trouve le premier port disponible à partir de 10000

	printf("Port : %d\n",port);

	if(listen(servsock, 1) == -1){//On indique que l'on peut seulement accepter qu'un seul client à se connecter.
		perror("listen()");
		exit(errno);
	}

	sockaddr_in caddin = { 0 };//On initialise l'addresse du client à zéro
	csize = sizeof(caddin);


	servfork = fork();//On créé un processus fils qui va gérer le serveur (reception de commande client et envoi de réponse)
	int statusCode = 0;

	do{
		if(servfork == 0 && isListened == 0){
			servfd = accept(servsock, (sockaddr *)&caddin, &csize);//On attend la connection d'un client dans le fork et on lui attribut un descripteur de fichier
			isListened = 1;

			if(servfd == -1)
			{
					perror("accept()");
					exit(errno);
			}

		}
		if(isListened == 1 && servfork == 0){
			if (getcwd(currentDir, sizeof(currentDir)) != NULL){//On envoie le working directory du serveur et on l'envoie au client
			    write(servfd,currentDir,sizeof(currentDir));
			}
			else{
				strcat(currentDir,"ERREUR : GETCWD()\n");
				write(servfd,currentDir,sizeof(currentDir));
			}
		}

		if(servfork != 0){
			if (getcwd(currentDir, sizeof(currentDir)) != NULL){
			    printf("<%s>$ ",currentDir);
			}
			else{
				printf("ERREUR : GETCWD()\n");
			}

	    }

   		char caractere;
   		int index = 0;
   		int nbCaractere = 0;
   		int echappemment = 0;

		char *arguments[20];
		int nbArguments = 0;

		int nbCmdPipe = 1;
		int positionPipe[20];
		int typePipe[20];
		positionPipe[0] = 0;

		if(servfork != 0){
	   		while((caractere = getchar()) != '\n'){ //Récupération des caractères fournis par l'utilisateur

	   			if(caractere == '\\'){ //Détection du caractère d'échappement
   					echappemment = 1;
   				}
   				else{
		   			if((echappemment == 0 && caractere == ' ' && nbCaractere > 0) || caractere == '\n'){ //Si on passe à un nouvel argument


		   				commande[index] = '\0'; //Ajout ud caractère de fin de chaîne
		   				arguments[nbArguments] = &(commande[index - nbCaractere]); //On pointe vers l'arguments
		   				nbArguments++; //On indique qu'on à un argument en plus
		   				nbCaractere = 0; //Reset du compteur de caractères
		   			}
		   			else{
						//Ajout du caractère dans la chaine qui contient la commande
		   				if(nbCaractere > 0 || caractere != ' '){

		   					commande[index] = caractere; 
		   					nbCaractere++;
		   				}
		   			}

		   			if(caractere == '|'){ //Détection du pipe
						positionPipe[nbCmdPipe] = nbArguments + 1; //Sauvegarde de la position du pipe
	   					typePipe[nbCmdPipe-1] = 0; //Sauvegarde du type de redirection
	   					nbCmdPipe++;
	   				}
	   				else if(caractere == '>'){ //Détection de la redirection de la sortie standard
	   					if(nbCaractere == 1){ //Détection de la redirection 1 de la sortie standard
	   						positionPipe[nbCmdPipe] = nbArguments + 1; //Sauvegarde de la position de la redirection
	   						typePipe[nbCmdPipe-1] = 1; //Sauvegarde du type de redirection
	   						nbCmdPipe++;
	   					}
	   					else if(nbCaractere == 2 && commande[index - 1] == '>' && typePipe[nbCmdPipe-2] == 1){ //Détection de la redirection 2 de la sortie standard
	   						typePipe[nbCmdPipe-2] = 2; //Sauvegarde du type de redirection
	   					}
	   				}
	   				else if(caractere == '<'){ //Détection de la redirection de l'entrée standard
	   					if(nbCaractere == 1){ //Détection de la redirection 1 de l'entrée standard
	   						positionPipe[nbCmdPipe] = nbArguments + 1; //Sauvegarde de la position de la redirection
	   						typePipe[nbCmdPipe-1] = 3; //Sauvegarde du type de redirection
	   						nbCmdPipe++;
	   					}
	   					else if(nbCaractere == 2 && commande[index - 1] == '<' && typePipe[nbCmdPipe-2] == 3){ //Détection de la redirection 2 de l'entrée standard
	   						typePipe[nbCmdPipe-2] = 4; //Sauvegarde du type de redirection
	   					}
	   				}
	   				index++;
   				}
	   		}
	   	}
   		else if(isListened){
   			int n = 0;
   			while((n=(recv(servfd, &caractere, 1 ,0)))>0){
   				if(caractere == '\n'){//Si on rencontre le caractère de fin de ligne, on arrête de lire la commande client.
   					break;
   				}
   				if(caractere == '\\'){ //Détection du caractère d'échappement
   					echappemment = 1;
   				}
   				else{
		   			if((echappemment == 0 && caractere == ' ' && nbCaractere > 0) || caractere == '\n'){ //Si on passe à un nouvel argument


		   				commande[index] = '\0'; //Ajout ud caractère de fin de chaîne
		   				arguments[nbArguments] = &(commande[index - nbCaractere]); //On pointe vers l'arguments
		   				nbArguments++; //On indique qu'on à un argument en plus
		   				nbCaractere = 0; //Reset du compteur de caractères
		   			}
		   			else{
						//Ajout du caractère dans la chaine qui contient la commande
		   				if(nbCaractere > 0 || caractere != ' '){

		   					commande[index] = caractere; 
		   					nbCaractere++;
		   				}
		   			}

		   			if(caractere == '|'){ //Détection du pipe
						positionPipe[nbCmdPipe] = nbArguments + 1; //Sauvegarde de la position du pipe
	   					typePipe[nbCmdPipe-1] = 0; //Sauvegarde du type de redirection
	   					nbCmdPipe++;
	   				}
	   				else if(caractere == '>'){ //Détection de la redirection de la sortie standard
	   					if(nbCaractere == 1){ //Détection de la redirection 1 de la sortie standard
	   						positionPipe[nbCmdPipe] = nbArguments + 1; //Sauvegarde de la position de la redirection
	   						typePipe[nbCmdPipe-1] = 1; //Sauvegarde du type de redirection
	   						nbCmdPipe++;
	   					}
	   					else if(nbCaractere == 2 && commande[index - 1] == '>' && typePipe[nbCmdPipe-2] == 1){ //Détection de la redirection 2 de la sortie standard
	   						typePipe[nbCmdPipe-2] = 2; //Sauvegarde du type de redirection
	   					}
	   				}
	   				else if(caractere == '<'){ //Détection de la redirection de l'entrée standard
	   					if(nbCaractere == 1){ //Détection de la redirection 1 de l'entrée standard
	   						positionPipe[nbCmdPipe] = nbArguments + 1; //Sauvegarde de la position de la redirection
	   						typePipe[nbCmdPipe-1] = 3; //Sauvegarde du type de redirection
	   						nbCmdPipe++;
	   					}
	   					else if(nbCaractere == 2 && commande[index - 1] == '<' && typePipe[nbCmdPipe-2] == 3){ //Détection de la redirection 2 de l'entrée standard
	   						typePipe[nbCmdPipe-2] = 4; //Sauvegarde du type de redirection
	   					}
	   				}
	   				index++;
				}
   			}

   			if(n==-1){
   				isListened=0;
   			}
   		}

   		if(nbCaractere > 0){ //Si on un arguments sans caractère de fin de chaine
		   	commande[index] = '\0'; //On l'ajoute
		   	arguments[nbArguments] = &(commande[index - nbCaractere]); //On pointe vers l'arguments
	   		nbArguments++; //On indique qu'on à un argument en plus
	   	}

		int nbPipe = nbCmdPipe - 1; //On calcul le nombre de pipes

		if(nbArguments)
		if(isListened && servfork == 0 && ((strcmp(arguments[0],"disconnect") == 0) || (strcmp(arguments[0],"exit") == 0) || (strcmp(arguments[0],"connect") == 0))){
			//Si on reçoit une commande disconnect, exit, ou connect du client, on arrête de lire le client (isListened)
			isListened = 0;
		}

		if(servfork == 0 && isListened){//Si on est le fils, donc le processus gèrant le serveur et qu'on est encore écouté (pas de disconnect de la part du client)
			char name[10];
			int tube[2];
			pipe(tube);//On construit un pipe pour lier tube0 et tube1
			char endstr[2];
			snprintf(endstr, 2, "%c%c", 0, 0);//String nécessaire pour marquer la fin d'envoi de texte de la part du serveur au client
			strcpy(name, arguments[0]);

			dup2(tube[1],STDOUT_FILENO);//On remplace la sortie standard par notre tube1


			if(nbPipe > 0){ //Si on a des pipes

				int i;
				int status;
				int pid;
				int *pipes = malloc(sizeof(int) * (nbPipe*2));
				for(i = 0; i < nbPipe*2; i += 2){ //Initialisation des tubes
					pipe(pipes + i);
				}
				int numero = 0;
				int indexPipe = 0;

				while(numero < nbCmdPipe){ //Boucle pour lancer toutes les commandes avec les pipes

					pid = fork();
					if(pid == 0){
						int block = 0;

						char **args;
						int borneSup;
						int nbArgs;

						if(numero < nbCmdPipe - 1){ //Si ce n'est pas la dernière commande

							borneSup = positionPipe[numero+1] - 1; //On détermine la position du dernier argument
							nbArgs = positionPipe[numero+1] - positionPipe[numero]- 1; //Et leur nombre
						}
						else{ //Si c'est la dernière commande
							borneSup = nbArguments; //On détermine la position du dernier argument
							nbArgs = nbArguments - positionPipe[numero]; //Et leur nombre
						}


						arguments[borneSup] = NULL; //On remplace le pipe par NULL dans le tableau (nécessaire pour les execs)
						args = &arguments[positionPipe[numero]]; //On pointe vers le début des arguments de la commande

						char *name = args[0];

						char str[50];
						sprintf(str, "%s\n", name);

						//Si on va lancer su on ajoute le nom de l'exécutable en arguments
						if(strcmp(name, "su") == 0){
							arguments[nbArguments++] = argv[0];
						}

						arguments[nbArguments] = NULL;

						if(numero == ((indexPipe+1)*2)){
							indexPipe++;
						}
						
						int fd;
						//Si ce n'est pas la première commande
						if(numero != 0){

							//Redirection vers l'entrée standard de la commande suivante
							if(typePipe[indexPipe] == 0){
								dup2(pipes[(numero - 1) * 2], STDIN_FILENO);
							}
							else if(typePipe[indexPipe] == 1 || typePipe[indexPipe] == 2){
								block = 1;
							}	
						}

						//Si ce n'est pas la dernière commande
						if(numero < nbCmdPipe - 1){

							//Redirection de la sortie standard de la commande courante
							if(typePipe[indexPipe] == 0){
								dup2(pipes[(numero*2 + 1)], STDOUT_FILENO);
							}
							else if(typePipe[indexPipe] == 1 || typePipe[indexPipe] == 2){

								int flags;
								mode_t mode = S_IRWXU | S_IRWXG;

								if(typePipe[indexPipe] == 1){
									flags = O_WRONLY | O_CREAT;
								}
								else{

									flags = O_APPEND | O_CREAT | O_WRONLY;
								}

								char **args_2 = &arguments[positionPipe[numero+1]];
								char *file = args_2[0];

								if((fd = open(file, flags, mode)) < 0){
									fprintf(stderr, "Erreur : %s\n", strerror(errno));
									return -1;
								}
								dup2(fd, STDOUT_FILENO);
								close(fd);
							}
						}


						if(block == 0){
							//Fermeture de tous les fds utilisé par les pipes
							for(int j = 0; j < nbPipe*2; j++){
								close(pipes[j]);
							}
							
							//Exécution de la commande					
							return launchCmd(nbArgs, args, argv[0]);
						}
						else{
							return 0;
						}
					}
					numero++;
				}

				//Fermeture de tous les fds utilisé par les pipes
				for(i = 0; i < nbPipe*2; i++){
					close(pipes[i]);
				}

				//Attente de la fin de toutes les commandes
				for(i = 0; i < nbCmdPipe; i++){
					wait(&status);
				}

				free(pipes);
			}
			else{ //Si y a pas de pipes

				arguments[nbArguments] = NULL; //On ajoute NULL comme dernière argument pour les execs
				statusCode = launchCmd(nbArguments, arguments, argv[0]); //On exécute la commande
			}
			
			write(tube[1], endstr, 2);//On rajoute dans le tube1 le string de fin d'envoie (\0\0).
			close(tube[1]);//On ferme le tube d'envoie

			char c;
			char end = 0;
			while(((read(tube[0], &c, 1))>0)){
				send(servfd, &c, 1, 0);
				if(c == '\0'){//On repère si on trouve 2 caractère \0 consécutif et on arrête de lire quand on les trouve (string de fin d'envoie)
					if(end){
						break;
					}else{
						end = 1;
					}
				}else{
					end = 0;
				}
				
			}
			close(tube[0]);
			
		}else if(servfork != 0){//Si on est le père (ce n'est alors pas une demande cliente, mais une demande directe de l'utilisateur du terminal)
			if(nbPipe > 0){

				int i;
				int status;
				int pid;
				int *pipes = malloc(sizeof(int) * (nbPipe*2));
				for(i = 0; i < nbPipe*2; i += 2){ //Initialisation des tubes
					pipe(pipes + i);
				}
				int numero = 0;
				int indexPipe = 0;

				while(numero < nbCmdPipe){ //Boucle pour lancer toutes les commandes avec les pipes

					pid = fork();
					if(pid == 0){
						int block = 0;

						char **args;
						int borneSup;
						int nbArgs;

						if(numero < nbCmdPipe - 1){ //Si ce n'est pas la dernière commande

							borneSup = positionPipe[numero+1] - 1; //On détermine la position du dernier argument
							nbArgs = positionPipe[numero+1] - positionPipe[numero]- 1; //Et leur nombre
						}
						else{ //Si c'est la dernière commande
							borneSup = nbArguments; //On détermine la position du dernier argument
							nbArgs = nbArguments - positionPipe[numero]; //Et leur nombre
						}


						arguments[borneSup] = NULL; //On remplace le pipe par NULL dans le tableau (nécessaire pour les execs)
						args = &arguments[positionPipe[numero]]; //On pointe vers le début des arguments de la commande

						char *name = args[0];

						char str[50];
						sprintf(str, "%s\n", name);

						//Si on va lancer su on ajoute le nom de l'exécutable en arguments
						if(strcmp(name, "su") == 0){
							arguments[nbArguments++] = argv[0];
						}

						arguments[nbArguments] = NULL; //On ajoute NULL comme dernière argument pour les execs

						if(numero == ((indexPipe+1)*2)){
							indexPipe++;
						}
						
						int fd;
						//Si ce n'est pas la première commande
						if(numero != 0){

							//Redirection vers l'entrée standard de la commande suivante
							if(typePipe[indexPipe] == 0){
								dup2(pipes[(numero - 1) * 2], STDIN_FILENO);
							}
							else if(typePipe[indexPipe] == 1 || typePipe[indexPipe] == 2){
								block = 1;
							}	
						}

						//Si ce n'est pas la dernière commande
						if(numero < nbCmdPipe - 1){

							//Redirection de la sortie standard de la commande courante
							if(typePipe[indexPipe] == 0){
								dup2(pipes[(numero*2 + 1)], STDOUT_FILENO);
							}
							else if(typePipe[indexPipe] == 1 || typePipe[indexPipe] == 2){

								int flags;
								mode_t mode = S_IRWXU | S_IRWXG;

								if(typePipe[indexPipe] == 1){
									flags = O_WRONLY | O_CREAT;
								}
								else{

									flags = O_APPEND | O_CREAT | O_WRONLY;
								}

								char **args_2 = &arguments[positionPipe[numero+1]];
								char *file = args_2[0];

								if((fd = open(file, flags, mode)) < 0){
									fprintf(stderr, "Erreur : %s\n", strerror(errno));
									return -1;
								}
								dup2(fd, STDOUT_FILENO);
								close(fd);
							}
						}


						if(block == 0){
							//Fermeture de tous les fds utilisé par les pipes
							for(int j = 0; j < nbPipe*2; j++){
								close(pipes[j]);
							}
							
							//Exécution de la commande					
							return launchCmd(nbArgs, args, argv[0]);
						}
						else{
							return 0;
						}
					}
					numero++;
				}

				//Fermeture de tous les fds utilisé par les pipes
				for(i = 0; i < nbPipe*2; i++){
					close(pipes[i]);
				}

				//Attente de la fin de toutes les commandes
				for(i = 0; i < nbCmdPipe; i++){
					wait(&status);
				}

				free(pipes);
			}
			else{ //Si y a pas de pipes

				arguments[nbArguments] = NULL; //On ajoute NULL comme dernière argument pour les execs
				statusCode = launchCmd(nbArguments, arguments, argv[0]); //On exécute la commande
			}
		}
	}
	while(statusCode != 2); //On boucle tant qu'on a pas réçu la commande exit

		kill(servfork,SIGTERM);
		close(servfork);
		close(servsock);
		close(servfd);

		return 0;
}


//Fonction qui gère l'appel des commandes on fonction du type de compilation
int call(char *name, int argc, char *argv[20]){

	int statusCode = 0;

	if(TYPECOMPILATION == INCLUS && strcmp(name, "su") != 0){ //Compilation du type incluse

		pfCommande cmd = getCmd(name); //On récupère le pointeur de la fonction
		cmd(argc, argv); //Lancement de la commande
	}
	else if(TYPECOMPILATION == EXE || strcmp(name, "su") == 0){ //Compilation du type exécutable

		pid_t pid;
		pid = fork(); //Création d'un processus fils
		if(pid == 0){

			char cmd[100];
			sprintf(cmd, "%s/%s", EXEPATH, name); //On forme le chemin complet de l'exécutable (au cas où un cd aurait été effectué)

			statusCode = execv(cmd, argv); //Exécution de l'exécutable
			if(statusCode == -1){
				printf("ERREUR : %s\n", strerror(errno)); //Affichage des erreurs
			}
		}
		else{
			int status;
			wait(&status); //Attente de la fin du processus fils
		}
	}
	else if(TYPECOMPILATION == LIB){ //Compilation du type librairie

		pfCommande commande;
		void *lib;

		if((lib=dlopen("libCmds.so", RTLD_LAZY)) == NULL){ //Chargement de la libraire
			fprintf(stderr, "ERREUR : Library couldn't be found\n");
			return -1;
		}
		if((commande=(pfCommande)dlsym(lib, name)) == NULL){ //Récupération de la fonction dans la libraire
			fprintf(stderr, "ERREUR : Function couldn't be found\n");
			return -1;
		}

		commande(argc, argv); //Lancement de la commande
		dlclose(lib); //Fermeture de la librairie
	}

	return statusCode;
}

//Fonction qui renvoie les pointeurs de fonctions
pfCommande getCmd(char *name){

	pfCommande tabCmd[] = {cp, ls, mkd, pwd, cat, du, mv, rm, chmod1};
	char* tabNameCmd[] = {"cp", "ls", "mkdir", "pwd", "cat", "du", "mv" , "rm", "chmod"};

	//Recherche du pointeur de fonction et renvoi de celui-ci
	for(int i = 0; i < NBCOMMANDE - 1; i++){
		if(strcmp(name, tabNameCmd[i]) == 0){
			return tabCmd[i];
		}
	}

	return NULL;
}

//Fonction qui indique si une commade existe
int commandeExiste(char *name){

	char* tabNameCmd[] = {"cp", "ls", "mkdir", "pwd", "cat", "su", "du", "mv", "rm", "chmod"};

	for(int i = 0; i < NBCOMMANDE; i++){

		if(strcmp(name, tabNameCmd[i]) == 0){
			return 1;
		}
	}

	return 0;
}

//Fonction qui change le répertoire de travail
int cd(int argc, char *argv[]){

 	char *path;

 	if(argc > 1){ //On regarde si on un argument

		path = argv[1]; //Si oui c'est le nouveau répertoire de travail
 	}
 	else{

 		path = "/"; //Sinon c'est la racine
 	}

 	//Changement de répertoire
 	if(chdir(path) != 0){

 		printf("ERREUR : %s\n", strerror(errno)); //Affichage des erreurs
 		return -1;
 	}

 	return 0;
}

//Fonction qui indique le nom de l'utilisateur courant
int whoami(int argc, char *argv[]){

 	struct passwd *pw;
 	int uid = getuid(); //Récupération de l'uid

	pw = getpwuid(uid); //Récupération des information lié à l'uid

	if(pw == NULL){ //Si l'uid n'existe pas

		fprintf(stderr, "ERREUR : User doesn't exist\n"); //On affiche une erreur
		return -1;
	}
	else{ //Sinon

		char *username = pw->pw_name; //On récupère le nom de l'utilisateur

		printf("%s\n", username); //On l'affiche
	}

	return 0;
}


//Fonction qui affiche les commandes disponibles
int commandesDisponibles(int argc, char *argv[]){

	char* tabNameCmd[] = {"cp", "ls", "mkdir", "pwd", "cat", "su", "du", "mv", "rm", "chmod", "commands", "connect", "disconnect"};

	for(int i = 0; i < NBCOMMANDE + 3; i++){

		printf("- %s\n",tabNameCmd[i]);
	}

	return 0;
}


void disconnect(int signal){//Sighandler pour gérer la déconnection éventuel du terminal en cas d'un Ctrl+C si il est connecté à un autre terminal.
	int clisock = stockConnect(0, 0);//Récupération du socket client pour envoyer le disconnect au serveur et quitter normalement le programme.
	send(clisock,"disconnect",strlen("disconnect")+1,0);
	close(clisock);
	exit(0);
}

int stockConnect(char stock, int cSock){//Fonction pour stocker le descripteur de fichier client étant l'interface de connection.
	static int clisock = 0;
	if(stock){
		clisock = cSock;
	}else{
		return clisock;
	}
	return 0;

}

int pbconnect(int argc, char *argv[]){
	int clisock;
	if(argc != 3){
		printf("Usage : connect <addresse_ip> <port>\n");
		return -1;
	}else{
		clisock = socket(AF_INET, SOCK_STREAM, 0);//On créé le socket client (descripteur de fichier servant d'interface)

		if(clisock == -1)
		{
			perror("socket()");
			return -1;
		}

		hostent *hostinfo = NULL;
		sockaddr_in clin = { 0 };//On déclare et initialise l'addresse à laquel on va se connecter
		char *hostname = argv[1];

		hostinfo = gethostbyname(hostname);
		if (hostinfo == NULL)//On repère si l'host est récupérer
			{	
				fprintf (stderr, "Hôte inconnu : %s.\n", hostname);
				printf("ERROR : %s\n", strerror(errno));
				return -1;
		}

		clin.sin_addr = *(in_addr *) hostinfo->h_addr_list[0];//On donne l'addresse serveur
		clin.sin_port = htons(atoi(argv[2]));//On donne le port
		clin.sin_family = AF_INET;//On indique le protocol IPv4

		if(connect(clisock,(sockaddr *) &clin, sizeof(sockaddr)) == -1)
			{
			perror("connect()");
			printf("ERROR : %s\n", strerror(errno));
			return -1;
		}

		stockConnect(1,clisock);//On stock le socket client

		//Entrées commande
		char commande[256];
		char currentDir[256];
		do{
	    	while(read(clisock,currentDir,sizeof(currentDir))<0);//On attend la reception du working directory et on le récupère
	    	printf("<server:%s>$ ",currentDir);
	    	char caractere;
	   		int index = 0;
	   		char finCommande = 0;
			while((caractere = getchar()) != '\n'){//On envoit directement caractère par caractère la commande cliente au serveur
				send(clisock, &caractere, 1, 0);
				if(caractere == ' '){
					finCommande = 1;
				}
				if(!finCommande){
					commande[index]=caractere;
					index++;
				}
		   	}

		   	send(clisock, &caractere, 1, 0);//On envoit le caractère \n
			commande[index] = '\0';

			if((strcmp(commande,"disconnect") != 0) && (strcmp(commande,"connect") != 0) && (strcmp(commande,"exit") != 0) ){//Si on ne veut pas se déconnecter
				//printf("envoicommande\n");
				char c;
				char end = 0;
				while(recv(clisock, &c, 1, 0)>0){//On attend la réception du résultat serveur et on le print dans le terminal
					if(c == '\0'){
						if(end){
							break;
						}else{
							end = 1;
						}
					}else{
						if(end){
							printf("%c",0);
						}
						end = 0;
						printf("%c",c);
					}
				}
				
			}
			else{
				break;
			}
		}while((strcmp(commande, "exit") != 0) );	

	}
	close(clisock);

	return 0;
}


//Fonction qui lance les commandes
int launchCmd(int nbArguments, char *arguments[20], char *exeName){

	int statusCode = 0;
	char name[10];

	strcpy(name, arguments[0]);

	//Si on va lancer su on ajoute le nom de l'exécutable en arguments
	if(strcmp(name, "su") == 0){
		arguments[nbArguments++] = exeName;
	}

	arguments[nbArguments] = NULL; //On ajoute NULL comme dernière argument pour les execs

	if(commandeExiste(name) == 1 || name[0] == '.' || name[0] == '/'){ //Si c'est une commande système ou un exécutable utilisateur

		statusCode = call(name, nbArguments, arguments);
	}
	else if(strcmp(name, "cd") == 0){ //Si c'est une commande de l'interpréteur : cd

		statusCode = cd(nbArguments, arguments);
	}
	else if(strcmp(name, "whoami") == 0){ //Si c'est une commande de l'interpréteur : whoami

		statusCode = whoami(nbArguments, arguments);
	}
	else if(strcmp(name, "commands") == 0){ //Si c'est une commande de l'interpréteur : commands
		statusCode = commandesDisponibles(nbArguments, arguments);
	}
	else if(strcmp(name, "connect") == 0){ //Si c'est une commande de l'interpréteur : connect

		pbconnect(nbArguments, arguments);
	}
	else if(strcmp(name, "exit") == 0){ //Si c'est une commande de l'interpréteur : exit
		return 2;
	}
	else{ //Si la commande exite pas
		fprintf(stderr, "Commande %s non implémenté\n", name);
		statusCode = -1;
	}

	return statusCode;
}