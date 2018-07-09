#define _DEFAULT_SOURCE // Pour utiliser la fonction lstat
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include "du.h"


// Structure qui va contenir les flags à destination des options
struct SFlags
{
  int flag_s ;    // Option pour faire la somme de tous les fichiers et sous répertoires d'un répertoire
  int flag_S ;    // Option pour faire la somme de tous les fichiers d'un répertoire, excluant les sous répertoires
  int flag_b ;    // Option pour afficher la taille en bytes
  int flag_a ;    // Option pour afficher des informations sur tous les fichiers (dont ceux qui ne sont pas répertoires)
  int flag_c ;    // Afficher un total d'usage disque à partir des autres arguments
  int flag_L ;    // Calcule l'usage disque pour les références des liens rencontrés
};

#ifdef EXE

int main(int argc, char *argv[]){
  int statusCode;
  statusCode = du(argc, argv);
  return statusCode;
}

#endif

/**
 * Affiche la taille de fichiers donnés.
 * @param  argc le nombre d'arguments passés à la fonction.
 * @param  argv les différents arguments passés à la fonction.
 * @return 0 si l'opération s'est bien passée, -1 sinon.
 */
int du(int argc, char *argv[]){

  // Flags à destination des options
  SFlags *mes_flags = malloc(sizeof(SFlags));
  mes_flags->flag_s = 0;
  mes_flags->flag_S = 0;
  mes_flags->flag_a = 0;
  mes_flags->flag_c = 0;
  mes_flags->flag_L = 0;
  mes_flags->flag_b = 0;


  int nb_fichiers = 0; 		// Nombre de chemins de fichiers / dossiers en arguments de la commande

  // On récupère les options de notre commande
  for(int i = argc-1; i >= 1; i--){
    // Option rencontrée
    if(argv[i][0] == '-'){
      {
        for (unsigned int j = 1; j < strlen(argv[i]); j++) {
          switch (argv[i][j]) {
            case 's':
            mes_flags->flag_s = 1;
            break;
            case 'S':
            mes_flags->flag_S = 1;
            break;
            case 'b':
            mes_flags->flag_b = 1;
            break;
            case 'a':
            mes_flags->flag_a = 1;
            break;
            case 'c':
            mes_flags->flag_c = 1;
            break;
            case 'L':
            mes_flags->flag_L = 1;
            break;
            default:
            fprintf(stderr, "cp: Option %c invalide.\n", argv[i][j]);
            free(mes_flags);
            return -1;
          }
        }
      }
    }
    // Chemin rencontré
    else{
      nb_fichiers++;
    }
  }

  // Il faut au moins un fichier cible
  if(nb_fichiers == 0){
    free(mes_flags);
    fprintf(stderr, "du: Veuillez indiquer au moins 1 fichier cible en argument.\n");
    return -1;
  }

  // Les options -a et -s sont incompatibles
  if(mes_flags->flag_a == 1 && mes_flags->flag_s == 1){
    free(mes_flags);
    fprintf(stderr, "du: impossible d'afficher à la fois un résumé et toutes les entrées.\n");
    return -1;
  }


  char *current_path;
  int nb_fichiers_traites = 0;
  int i = 1;
  int val_courante = 0;
  int total = 0;

  while(i < argc && nb_fichiers_traites < nb_fichiers){
    // Si ce n'est pas une option, ce doit être un chemin
    if(argv[i][0] != '-'){

      if(mes_flags->flag_a == -1){ // On a parcouru un dossier auparavant avec l'option -a désactivée
        mes_flags->flag_a = 0;
      }

      nb_fichiers_traites++;
      current_path = argv[i];

      val_courante = get_size(current_path, mes_flags);
      // Si l'option -s est activée, on n'affiche qu'une seule ligne pour le répertoire (et pas le détail)
      if(mes_flags->flag_s == 1){
        printf("%d       %s\n", val_courante, current_path);
      }
      total += val_courante;
    }
    i++;
  }
  // On affiche le total des tailles des arguments
  if(mes_flags->flag_c == 1){
    printf("%d       %s\n", total, "total");
  }
  free(mes_flags);
  return 0;
}

/**
 * Retourne la taille d'un fichier / dossier passé en paramètre
 * @param  current_path le chemin du fichier concerné
 * @param  mes_flags    les flags liés aux options de la commande
 * @return la taille du fichier, 0 s'il n'existe pas
 */
int get_size(char *current_path, SFlags *mes_flags){

  Stat *stat_cur = malloc(sizeof(Stat));
  int size = 0;
  int etat_stat = -1;

  // Si on souhaite connaître la taille des fichiers pointés par les liens plutôt que la taille des liens
  if(mes_flags->flag_L == 1){
    etat_stat = stat(current_path, stat_cur);
  }else{
    etat_stat = lstat(current_path, stat_cur);
  }

  if (etat_stat != 0){
    fprintf(stderr, "du : impossible d'évaluer '%s': Aucun fichier ou dossier de ce type.\n", current_path);
    free(stat_cur);
    return 0; // Le fichier n'existe pas
  }
  // Si c'est un répertoire
  else if (S_ISDIR(stat_cur->st_mode)){
    size =  get_size_dossier(current_path, stat_cur, mes_flags);
  }
  else{
    size = get_size_fichier(current_path, stat_cur, mes_flags);
  }
  free(stat_cur);
  return size;
}

/**
 * Permet d'obtenir la taille d'un fichier dont le chemin est passé en paramètre
 * @param  current_path le chemin du fichier concerné
 * @param  stat_cur     une structure contenant des informations sur le fichier
 * @param  mes_flags    les flags liés aux options de la commande
 * @return la taille du fichier, 0 s'il n'existe pas
 */
int get_size_fichier(char *current_path, Stat *stat_cur, SFlags *mes_flags){

  int size = stat_cur->st_blocks;

  // Taille en bytes
  if(mes_flags->flag_b == 1){
    size = stat_cur->st_size;
  }

  // On affiche la taille si l'option a est activée et que l'option s est désactivée
  if(mes_flags->flag_a != -1 && mes_flags->flag_s == 0){
    printf("%d       %s\n", size, current_path);
  }
  return size;
}

/**
 * Permet d'obtenir la taille d'un dossier dont le chemin est passé en paramètre
 * @param  current_path le chemin du fichier concerné
 * @param  stat_cur     une structure contenant des informations sur le fichier
 * @param  mes_flags    les flags liés aux options de la commande
 * @return la taille du fichier, 0 s'il n'existe pas
 */
int get_size_dossier(char *current_path, Stat *stat_cur, SFlags *mes_flags){

  if(mes_flags->flag_a == 0){
    mes_flags->flag_a = -1;
  }

  int size = stat_cur->st_blocks;

  // Taille en bytes
  if(mes_flags->flag_b == 1){
    size = stat_cur->st_size;
  }

  DIR *dirp;
  struct dirent *dptr;

  char* chemin_fic_courant;

  // Ouverture du dossier en vue du parcours de son contenu
  if((dirp = opendir(current_path)) == NULL){
    fprintf(stderr, "cp: impossible d'ouvrir le dossier '%s'.\n", current_path);
    return 0;
  }

  int i = 0;

  while((dptr = readdir(dirp)) != NULL){
    // Si le fichier courant ne correspond ni au dossier courant, ni au parent
    if(strcmp(dptr->d_name, ".") != 0 && strcmp(dptr->d_name, "..") != 0){

      // On supprime les éventuels slashs présents à la fin du path
      i = strlen(current_path)-1;
      while(current_path[i] == '/'){
        current_path[i] = '\0';
        i--;
      }
      chemin_fic_courant = malloc(sizeof(char)*(2+strlen(current_path)+strlen(dptr->d_name)));
      sprintf(chemin_fic_courant, "%s/%s", current_path, dptr->d_name); // Reconstitution du chemin du fichier source

      // Si le flag S est activée, on ne prend plus en compte la taille des sous-répertoires du dossier traité
      if(mes_flags->flag_S == 1 && dptr->d_type == DT_DIR){
        get_size(chemin_fic_courant, mes_flags);
      }else{
        size += get_size(chemin_fic_courant, mes_flags);
      }
      free(chemin_fic_courant);

    }

  }

  closedir(dirp);
  // La taille des sous-répertoires est affichée par défaut, cachée si l'option -s est activée
  if(mes_flags->flag_s == 0){
    printf("%d       %s\n", size, current_path);
  }

  return size;
}
