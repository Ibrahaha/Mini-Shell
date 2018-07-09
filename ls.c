#define _GNU_SOURCE
#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "ls.h"


#ifdef EXE

int main(int argc, char *argv[]){

	int statusCode = 0;
	statusCode = ls(argc, argv);
	return statusCode;
}

#endif

int ls(int argc, char *argv[]){	

	char *path = ".";
	int bool_A = 0;
	int bool_L = 0;
	int i;
	//Récupération des options et du répertoire cible
	if(argc > 1){		
		char *arg_1 = argv[1];
		if(arg_1[0] == '-'){
			for(i = 1; i < strlen(arg_1); i++){
				
				if(arg_1[i] == 'a'){
					bool_A = 1;
				}
				else if(arg_1[i] == 'l'){
					bool_L = 1;
				}
			}
			if(argc > 2){
				path = argv[2];
			}
		}
		else{
			path = argv[1];
		}
	}	

	DIR *dirp;  //crée un pointeur de type repertoire
	struct dirent *dptr;  // crée une structure entrée-repertoire
	struct stat st;

	if((dirp=opendir(path))==NULL){ //Ouverture du répertoire
		
		fprintf(stderr, "ERREUR : Répertoire absent\n");
		return -1;
	}

	while((dptr=readdir(dirp)) != NULL){ //Récupération des informations sur le contenu du répertoire
		if(!bool_L){ //Pas d'option l --> pas de détail
			if(bool_A || dptr->d_name[0] != '.'){ //On regarde si on doit afficher les fichiers avec un . (option a)
				char *msg = malloc(strlen(dptr->d_name) + 3);
				sprintf(msg, "%s  ", dptr->d_name);
				write(STDOUT_FILENO, msg, strlen(msg));
				free(msg);
			}
		}
		else{
			char *pathFile = malloc(sizeof(char) * strlen(path) + strlen(dptr->d_name) + 2);
			sprintf(pathFile, "%s/%s", path, dptr->d_name);
			lstat(pathFile, &st); //Récupération des informations sur le fichier courant
			if(bool_A || dptr->d_name[0] != '.'){ //On regarde si on doit afficher les fichiers avec un . (option a)
				//Affichage des informations sur le type de fichier
				if((st.st_mode & S_IFSOCK) == S_IFSOCK) write(STDOUT_FILENO, "s", strlen("s"));
				else if((st.st_mode & S_IFLNK) == S_IFLNK) write(STDOUT_FILENO, "l", strlen("l"));
				else if((st.st_mode & S_IFREG) == S_IFREG) write(STDOUT_FILENO, "-", strlen("-"));
				else if((st.st_mode & S_IFBLK) == S_IFBLK) write(STDOUT_FILENO, "b", strlen("b"));
				else if((st.st_mode & S_IFDIR) == S_IFDIR) write(STDOUT_FILENO, "d", strlen("d"));
				else if((st.st_mode & S_IFCHR) == S_IFCHR) write(STDOUT_FILENO, "c", strlen("c"));
				else if((st.st_mode & S_IFIFO) == S_IFIFO) write(STDOUT_FILENO, "p", strlen("p"));

				//Affichage des information sur le droits
				(st.st_mode & S_IRUSR) == S_IRUSR ? write(STDOUT_FILENO, "r", strlen("r")) : write(STDOUT_FILENO, "-", strlen("-"));
				(st.st_mode & S_IWUSR) == S_IWUSR ? write(STDOUT_FILENO, "w", strlen("w")) : write(STDOUT_FILENO, "-", strlen("-"));
				(st.st_mode & S_IXUSR) == S_IXUSR ? write(STDOUT_FILENO, "x", strlen("x")) : write(STDOUT_FILENO, "-", strlen("-"));

				(st.st_mode & S_IRGRP) == S_IRGRP ? write(STDOUT_FILENO, "r", strlen("r")) : write(STDOUT_FILENO, "-", strlen("-"));
				(st.st_mode & S_IWGRP) == S_IWGRP ? write(STDOUT_FILENO, "w", strlen("w")) : write(STDOUT_FILENO, "-", strlen("-"));
				(st.st_mode & S_IXGRP) == S_IXGRP ? write(STDOUT_FILENO, "x", strlen("x")) : write(STDOUT_FILENO, "-", strlen("-"));

				(st.st_mode & S_IROTH) == S_IROTH ? write(STDOUT_FILENO, "r", strlen("r")) : write(STDOUT_FILENO, "-", strlen("-"));
				(st.st_mode & S_IXOTH) == S_IXOTH ? write(STDOUT_FILENO, "w", strlen("w")) : write(STDOUT_FILENO, "-", strlen("-"));
				(st.st_mode & S_IWOTH) == S_IWOTH ? write(STDOUT_FILENO, "x", strlen("x")) : write(STDOUT_FILENO, "-", strlen("-"));


				//Affichage du nombre de lien dur
				char msg[50];
				sprintf(msg, " %lu", st.st_nlink);
				write(STDOUT_FILENO, msg, strlen(msg));
				
				//Affichage du nom d'utilisateur du propriétaire
				struct passwd *userInfo = getpwuid(st.st_uid);
				sprintf(msg, " %s", userInfo->pw_name);
				write(STDOUT_FILENO, msg, strlen(msg));

				//Affichage du nom de groupe du propriétaire
				struct group *groupInfo = getgrgid(st.st_gid);
				sprintf(msg, " %s", groupInfo->gr_name);
				write(STDOUT_FILENO, msg, strlen(msg));

				//Affichage de la taille
				sprintf(msg, " %ld", st.st_size);
				write(STDOUT_FILENO, msg, strlen(msg));

				//Affichage de la date de dernière modification
				struct tm *timeInfo = localtime(&st.st_mtime);
				sprintf(msg, " %4d-%02d-%02d %02d:%02d", timeInfo->tm_year + 1900, timeInfo->tm_mon + 1, timeInfo->tm_mday, timeInfo->tm_hour, timeInfo->tm_min);
				write(STDOUT_FILENO, msg, strlen(msg));

				//Affichage du nom de fichier
				sprintf(msg, " %s\n", dptr->d_name);
				write(STDOUT_FILENO, msg, strlen(msg));
			}

			free(pathFile);
		}
	}
	if(!bool_L){
		write(STDOUT_FILENO, "\n", strlen("\n"));
	}
	closedir(dirp);
	return 0;
}