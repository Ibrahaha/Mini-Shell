#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include"rm.h"

#ifdef EXE

int main(int argc, char *argv[]){

	int statusCode = 0;
	statusCode = rm(argc, argv);
	return statusCode;
}

#endif

// Permet de savoir si un machin dont le chemin est passé en paramètre est un dossier  (0 (n'existe pas), 1 (non fichier), 2 (fichier))
int est_quoi(char *path){

  struct stat st;
  if(lstat(path, &st) == -1){ 
    fprintf(stderr, "Erreur : %s\n", strerror(errno));
    return -1;
  }

  if((st.st_mode & S_IFSOCK) == S_IFSOCK) return 1;
  else if((st.st_mode & S_IFLNK) == S_IFLNK) return 1;
  else if((st.st_mode & S_IFREG) == S_IFREG) return 1;
  else if((st.st_mode & S_IFBLK) == S_IFBLK) return 1;
  else if((st.st_mode & S_IFDIR) == S_IFDIR) return 2;
  else if((st.st_mode & S_IFCHR) == S_IFCHR) return 1;
  else if((st.st_mode & S_IFIFO) == S_IFIFO) return 1;

  return 0;
}



int rm(int argc, char *argv[]){

  int rflag  = 0; // Option pour réaliser les suppression de manière récursive (pour la suppression des dossiers)
  int nb_file = 0; // Nombre de chemins de fichiers / dossiers en arguments de la commande
  char *file[20];

  // Il faut au moins 1 arguments
  if(argc < 2){
    fprintf(stderr, "rm : pas d'arguments.\n");
    return -1;
  }

  // On récupère les options de notre commande
  for(int i = argc-1; i >= 1; i--){
    // Option rencontrée
    if(argv[i][0] == '-'){
      for (unsigned int j = 1; j < strlen(argv[i]); j++) {
        switch (argv[i][j]) {
          case 'r':
          rflag = 1;
          break;
          default:
          fprintf(stderr, "rm : Option %c invalide.\n", argv[i][j]);
          break;
        }
      }
    }
    // Chemin rencontré
    else{

      file[nb_file] = argv[i];
      nb_file++;
    }
  }


  for(int i = 0; i < nb_file; i++){

    if(rflag == 1){
      suppressionOptionR(file[i]);
    }
    else{
      suppression(file[i]);
    }
  }
  return 0;
}

int suppression(char *file){

  int nature = est_quoi(file);  // Indique la nature de la destination : -1 (non définie), 0 (n'existe pas), 1 (fichier), 2 (dossier), 3 (autre)

  switch (nature) {
    case 0:
      fprintf(stderr, "Votre fichier %s n'existe pas\n", file);
      return -1;
    break;
    case 1:
      if(unlink(file) == -1){ //suppression du fichier
        fprintf(stderr, "Erreur : %s\n", strerror(errno));
        return -1;
      }
      break;
    case 2:
      if(rmdir(file) == -1){ //suppression du dossier
        fprintf(stderr, "Erreur : %s\n", strerror(errno));
        return -1;
      }
      break;
  }

  return 0;
}

int suppressionOptionR(char *file){

  int nature = est_quoi(file);

  if(nature == 1){

    return suppression(file);
  }
  else if(nature != -1){

    DIR *dirp;  //crée un pointeur de type repertoire
    struct dirent *dptr;  // crée une structure entrée-repertoire
    if((dirp=opendir(file))==NULL){
    
      fprintf(stderr, "ERREUR : Répertoire %s absent\n", file);
      return -1;
    }
    else{
      while ((dptr = readdir(dirp)) != NULL) {
        char *new_file = dptr->d_name;
        if(new_file[0] != '.'){

          char *pathFile = malloc(sizeof(char) * (strlen(file) + strlen(new_file) + 2));
          sprintf(pathFile, "%s/%s", file, new_file);
          suppressionOptionR(pathFile);
          free(pathFile);
        }
      }
      closedir(dirp);
      suppression(file);
    }
  }
  else{
    return -1;
  }

  return 0;

}