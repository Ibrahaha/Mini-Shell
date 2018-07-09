#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <libgen.h>
#include <math.h>
#include <stdbool.h>
#include <ctype.h>
#include "chmod.h"

#ifdef EXE

int main(int argc, char *argv[]){
  int statusCode;
  statusCode = chmod1(argc, argv);
  return statusCode;
}

#endif

//conversion d'un octal en decimal
int conversion_decimal(int n) {
    int decimal=0;
    int i=0;
    while (n!=0) {
        int rem = n%10; //reste de la division euclidienne
        n/=10; 
        decimal += rem*pow(8,i);
        ++i;
    }
    return decimal; //on retourne le nombre passé en argument en decimal
}



//Permet le parcours du contenu d'un repertoire pour la commande
void parcours(char *chemin, int options, int permissions, Flags *flags)
{

	DIR *dirp_src;
    struct dirent *dptr_src;

    if ((dirp_src = opendir(chemin)) == NULL) {
        fprintf(stderr, "Erreur: %s\n",strerror(errno));
        free(flags);
        exit(EXIT_FAILURE);
    }

    while ((dptr_src = readdir(dirp_src)) != NULL) {

        if ((strcmp(dptr_src->d_name, "..") != 0) && (strcmp(dptr_src->d_name, ".") != 0)) {

            char fils_chemin[strlen(chemin) + strlen(dptr_src->d_name) + 2];
            strcpy(fils_chemin, chemin);
            strcat(fils_chemin, "/");
            strcat(fils_chemin, dptr_src->d_name);

            modif_perm(fils_chemin, options, permissions, flags); //on modifie les droits de l'élément
        }
    }

    if (closedir(dirp_src) != 0) {
        perror("Erreur fermeture repertoire");
        free(flags);
        exit(EXIT_FAILURE);
    }

}


//Permet la modification des permissions du fichier ou repertoire sis à l'adresse chemin
void modif_perm(char *chemin, int options, int permissions, Flags *flags)
{

	//on regarde les informationss sur l'élément
    struct stat st;
    if (stat(chemin, &st) == -1) 
    {
        if (errno == ENOENT) 
        {
            perror("Ce fichier n'existe pas");
            free(flags);
            exit(EXIT_FAILURE);
        }
    }
    //si on ne rencontre pas  de -f, on vérifie que l'élement est accessible en écriture avant de modifier ses droits
    char ok = 'n';
    if ((options & flags->f_flag) != flags->f_flag) 
    {
        if (access(chemin, W_OK) != 0)
         {
            //demande yes/no
            printf("Override permissions for %s ? [y/n] ", chemin);
            ok = getchar();
            printf("\n");
            fseek(stdin, 0L, SEEK_END);
        } 
        else {
            ok = 'y';
        }
    } 
    else 
    {
        ok = 'y';
    }
    if (ok == 'y') 
    {
        //on verifie si on est sur un dossier,
        if (S_ISDIR(st.st_mode)) 
        {

            //On verifie si on a l'option -r
            if ((options & flags->r_flag) == flags->r_flag) 
            {
                //modification des droits du contenu
                parcours(chemin, options, permissions, flags);
            }

        }

        if (chmod(chemin, (mode_t) permissions) == -1) 
        {
            fprintf(stderr, "Erreur : %s\n", strerror(errno));
            free(flags);
            exit(EXIT_FAILURE);
        }

    }

}




//la fonction d'entrée correspondant à la commande
int chmod1(int argc, char *argv[]) 
{

    int options = 0;
    int permissions = 0;
    optind = 1;

    Flags *flags = malloc(sizeof(Flags));

    char c;
    while((c = getopt(argc, argv, "Rvf")) != -1) 
    {
        switch(c) 
        {
            case 'R': //on veut modifier en récursif
                options |= 1;
                flags->r_flag = 1;
                break;
            case 'f':
                options |= 1;
                flags->f_flag = 1;
                break;
            case 'v':
                options |= 1;
                flags->v_flag = 1;
                break;
            default:
                fprintf(stderr,"option non recoonue!\n");
                free(flags);
                exit(EXIT_FAILURE);
        }
    }
    // on verifie egalement si le nombre d'argument est correct
    if (argc < 2) 
    {
        fprintf(stderr,"Error: arguments");
        free(flags);
        exit(EXIT_FAILURE);
    }
    permissions = atoi(argv[optind]); //on récupère les droits à la bonne place et on converit le string en integer à l'aide de atoi(const char*bidil)
    permissions = conversion_decimal(permissions);
    for (int i = optind+1; i < argc; i ++) 
    {
        modif_perm(argv[i], options, permissions, flags);
    }

    return 0;
}