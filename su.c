#define _GNU_SOURCE
#include <stdio.h>
#include <pwd.h>
#include <shadow.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>

#include "su.h"

int main(int argc, char *argv[]){

	int statusCode;
	statusCode = su(argc, argv);
	return statusCode;
}


int su(int argc, char *argv[]){

    struct passwd *pw;
    struct spwd *spwd;
    int statusCode;
    int uid, gid;
    char *password;
    char *username, *encrypted, *p;

    if(argc < 2) { //On regarde si un nom d'utilisateur a été fourni
        //Si non, on dit que l'utilisateur fourni est root
        uid = gid = 0;
        pw = getpwuid(0);
        username = pw->pw_name;
    }
    else{
        //Si oui
        username = argv[1];
        pw = getpwnam(username); //On récupère les informations sur l'utilisateur dans /etc/passwd
        if(pw == NULL){ //Si il existe pas

            fprintf(stderr, "ERREUR : User doesn't exist\n"); //On affiche une erreur
            return -1;
        }
        else{ //Sinon, on récupère

            uid = pw->pw_uid; //son uid
            gid = pw->pw_gid; //son gid
            password = pw->pw_passwd; //son mdp (qui n'est pas forcément renseigné)
        }
    }

    spwd = getspnam(username); //On récupère les informations sur l'utilisateur dans /etc/shadow
    if(spwd == NULL && errno == EACCES){ //On regarde si y pas eu d'erreur
        fprintf(stderr, "ERREUR : No permission to read shadow password file\n");
    }

    if(spwd != NULL){ //Si on a pu récupérer les informations dans /etc/shadow
        password = spwd->sp_pwdp; //On récupère le mdp crypté
    }

    //On demande le mot de passe
    char *askPass = "Mot de passe : ";
    char *passwordUser = getpass(askPass);

    encrypted = crypt(passwordUser, password); //On crypte le mot de passe fourni
    //On efface celui qui est non crytpé
    for(p = passwordUser; *p != '\0'; ){
        *p++ = '\0';
    }

    if(encrypted == NULL){ //On regarde si le cryptage c'est bien passé
        fprintf(stderr, "ERREUR : Cryptage du mot de passe\n");
        return -1;
    }

    if(strcmp(encrypted, password) != 0){ //On compare le mdp fourni et celui de /etc/shadow
        fprintf(stderr, "ERREUR : Échec d'authentification\n");
        return -1;
    }

    if((statusCode = setreuid(uid, gid)) != 0){ //On change l'uid et le gid
        fprintf(stderr,"%s\n", strerror(errno));
    }

    //Puis on réexécute un termninal avec le nouvel uid
    pid_t pid;
    pid = fork();
    char *arguments[] = {argv[argc-1], "NO", NULL};
    if(pid == 0){

        char cmd[50];

        sprintf(cmd, "%s/%s", EXEPATH, argv[argc-1]);

        statusCode = execv(cmd, arguments);
        if(statusCode == -1){
            fprintf(stderr, "ERREUR : %s\n", strerror(errno));
        }
    }
    else{
        int status;
        wait(&status);
    }

    return statusCode;
}