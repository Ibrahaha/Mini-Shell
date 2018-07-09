#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include "mv.h"

// Structure qui va contenir les flags à destination des options
struct SFlags
{
  int flag_i ;    // Option pour demander à l'utilisation une confirmation avant d'écraser tout fichier existant
  int flag_u ;    // Option empêchant le déplacemement d'un fichier régulier qui écraserait un fichier destination existant ayant été modifié plus récemment
  int flag_v ;    // Option pour activer le mode "verbeux"
};


#ifdef EXE

int main(int argc, char *argv[]){
  int statusCode;
  statusCode = mv(argc, argv);
  return statusCode;
}

#endif

/**
 * Fonction de déplaçage ou de renommage des fichiers.
 * @param  argc le nombre d'arguments passés à la fonction.
 * @param  argv les différents arguments passés à la fonction.
 * @return 0 si l'opération s'est bien passée, -1 sinon.
 */
int mv(int argc, char *argv[]){

  // Flags à destination des options
  SFlags *mes_flags = malloc(sizeof(SFlags));
  mes_flags->flag_i = 0;
  mes_flags->flag_u = 0;
  mes_flags->flag_v = 0;

  int nb_fichiers = 0; 		// Nombre de chemins de fichiers / dossiers en arguments de la commande
  int nature_dest = -1; 	// Indique la nature de la destination : -1 (non définie), 0 (n'existe pas), 1 (non dossier), 2 (dossier)
  char *src_path;					// Chemin source
  char *dest_path;				// Chemin de destination

  // Il faut au moins 2 arguments
  if(argc < 3){
    fprintf(stderr, "mv : opérande de fichier manquant.\n");
    free(mes_flags);
    return -1;
  }

  // On récupère les options de notre commande
  for(int i = argc-1; i >= 1; i--){
    // Option rencontrée
    if(argv[i][0] == '-'){
      {
        for (unsigned int j = 1; j < strlen(argv[i]); j++) {
          switch (argv[i][j]) {
            case 'i':
            mes_flags->flag_i = 1;
            break;
            case 'u':
            mes_flags->flag_u = 1;
            break;
            case 'v':
            mes_flags->flag_v = 1;
            break;
            default:
            fprintf(stderr, "mv: Option %c invalide.\n", argv[i][j]);
            free(mes_flags);
            return -1;
          }
        }
      }
    }
    // Chemin rencontré
    else{
      // On n'a pas encore rencontré de path
      if(nature_dest == -1){
        dest_path = argv[i];
        // On regarde si cela correspond à un dossier existant ou non
        nature_dest = est_un_dossier(dest_path);
      }
      nb_fichiers++;
    }
  }

  // Il faut indiquer au moins deux chemins pour exécuter la commande
  if(nb_fichiers < 2){
    fprintf(stderr, "mv : opérande de fichier manquant.\n");
    free(mes_flags);
    return -1;
  }

  // Si on a plusieurs chemins sources, la seule destination possible est un répertoire existant
  if(nb_fichiers > 2 && nature_dest != 2){
    fprintf(stderr, "mv : la cible '%s' n'est pas un répertoire.\n", dest_path);
    free(mes_flags);
    return -1;
  }

  int nb_fichiers_traites = 1;	// Nombre de fichiers (ou dossiers) traités, 1 par défaut (le fichier / répertoire de destination)
  int i = 1;

  // Si le dernier argument est un chemin vers un répertoire existant
  if(nature_dest == 2){
    // On parcours les chemins précédents pour copier les fichiers dans le répertoire existant
    while(i < argc && nb_fichiers_traites < nb_fichiers){
      // Si ce n'est pas une option, ce doit être un chemin
      if(argv[i][0] != '-'){
        nb_fichiers_traites++;
        src_path = argv[i];

        if(deplacement_vers_dossier(src_path, dest_path, mes_flags) == -1){
          free(mes_flags);
          return -1;
        }

      }
      i++;
    }
  }
  // Le dernier argument n'est pas un chemin vers un répertoire existant
  else{
    while(i < argc){
      // Si ce n'est pas une option, ce doit être le chemin
      if(argv[i][0] != '-'){
        src_path = argv[i];
        break;
      }
    }
    // La cible est un fichier existant
    if(nature_dest == 1){
      if(deplacement_cible_existante(src_path, dest_path, mes_flags) == -1){
        free(mes_flags);
        return -1;
      }else{
        free(mes_flags);
        return 0;
      }
    }
    // La cible n'existe pas
    else{
      if(rename(src_path, dest_path) == -1){
        fprintf(stderr, "mv : Erreur rencontrée lors de l'opération.\n");
        free(mes_flags);
        return -1;
      }else{
        if(mes_flags->flag_v == 1){
          fprintf(stderr, "'%s' -> '%s'\n", src_path, dest_path);
        }
      }
    }
  }
  free(mes_flags);
  return 0;
}

/**
 * Permet de déplacer un fichier / dossier vers un dossier existant
 * @param  src_path  le chemin du fichier source
 * @param  dest_path le chemin du dossier de destination
 * @param  mes_flags les flags liés aux options de la commande
 * @return 0 si le déplacement a bien été réalisé, -1 sinon
 */
int deplacement_vers_dossier(char* src_path, char* dest_path, SFlags *mes_flags){

  // On enlève les éventuels slashs en fin de la source
  int cpt = strlen(src_path) - 1;
  while(src_path[cpt] == '/' && cpt >= 0){
    cpt--;
  }
  src_path[cpt+1] = '\0';


  char* chemin_final_dest;    // Chemin de destination finale

  struct stat src_stat, final_dest, dest_stat;

  // La source n'existe pas --> Erreur
  if(stat(src_path, &src_stat) != 0){
      fprintf(stderr, "mv : impossible d'évaluer '%s': Aucun fichier ou dossier de ce type.\n", src_path);
      return -1;
  }

  // Problème d'accès à la destination --> Erreur
  if(stat(dest_path, &dest_stat) != 0){
      fprintf(stderr, "mv : impossible d'évaluer '%s': Aucun fichier ou dossier de ce type.\n", dest_path);
      return -1;
  }

  // On vérifie que les deux chemins source et destination e pointent pas vers le même fichier
  if ((src_stat.st_dev == dest_stat.st_dev) && (src_stat.st_ino == dest_stat.st_ino)) {
    fprintf(stderr, "mv: impossible de déplacer  '%s' vers un sous-répertoire de lui-même, %s.\n", src_path, dest_path);
    return -1;
  }

  chemin_final_dest = creer_chemin_final(src_path, dest_path);

  // Si un fichier porte déjà le même nom dans la destination
  if(stat(chemin_final_dest, &final_dest) == 0){

    int res = deplacement_cible_existante(src_path, chemin_final_dest, mes_flags);
    free(chemin_final_dest);
    return res;

  }
  // Si aucun fichier ne porte déjà le même nom -> simple déplacement du fichier source dans la destination
  else{
    if(rename(src_path, chemin_final_dest) == -1){
      fprintf(stderr, "mv : Erreur rencontrée lors de l'opération.\n");
      free(chemin_final_dest);
      return -1;
    }else{
      if(mes_flags->flag_v == 1){
        fprintf(stderr, "'%s' -> '%s'\n", src_path, chemin_final_dest);
      }
    }
  }
  free(chemin_final_dest);
  return 0;
}

/**
 * Déplacer un fichier / dossier vers une cible existante
 * @param  src_path  le chemin du fichier source
 * @param  dest_path le chemin du dossier de destination
 * @param  mes_flags les flags liés aux options de la commande
 * @return 0 si le déplacement a bien été réalisé, -1 sinon
 */
int deplacement_cible_existante(char* src_path, char* dest_path, SFlags *mes_flags){

  struct stat src_stat, dest_stat;

  // La source n'existe pas --> Erreur
  if(stat(src_path, &src_stat) != 0){
      fprintf(stderr, "mv : impossible d'évaluer '%s': Aucun fichier ou dossier de ce type.\n", src_path);
      return -1;
  }
  // Problème d'accès à la destination --> Erreur
  if(stat(dest_path, &dest_stat) != 0){
      fprintf(stderr, "mv : impossible d'évaluer '%s': Aucun fichier ou dossier de ce type.\n", dest_path);
      return -1;
  }

  // On vérifie que les deux chemins ne pointent pas vers le même fichier
  if ((src_stat.st_dev == dest_stat.st_dev) && (src_stat.st_ino == dest_stat.st_ino)) {
    fprintf(stderr, "mv: '%s' et './%s' identifient le même fichier.\n", src_path, dest_path);
    return -1;
  }

  // Si ce sont deux fichiers ou deux dossiers, on va écraser le fichier/dossier destination
  if ((S_ISDIR(src_stat.st_mode) && S_ISDIR(dest_stat.st_mode)) || (! S_ISDIR(src_stat.st_mode) && ! S_ISDIR(dest_stat.st_mode))){

    // Si l'option i est activée --> demande à l'utilisateur s'il souhaite écraser le fichier destination
    if(mes_flags->flag_i == 1){
      fprintf(stderr, "mv : voulez-vous écraser '%s' ? (y/n)\n", dest_path);
      char reponse = getchar();
      // si l'utilisateur saisit autre chose que 'y', la copie n'est pas effectuée
      if (reponse != 'y'){
        return 0;
      }
    }

    // Si le fichier de destination a été modifié plus tard que celui d'origine --> on annule le déplacement (ne concerne pas les dossiers)
    if(mes_flags->flag_u == 1){
      if(dest_stat.st_mtime > src_stat.st_mtime){
        if(! S_ISDIR(src_stat.st_mode) && ! S_ISDIR(dest_stat.st_mode)){
          fprintf(stderr, "mv : Le fichier de destination est plus récent que celui d'origine : déplacement annulé.\n");
          return -1;
        }
      }
    }
    // Déplacement fichier source vers la destination
    if(rename(src_path, dest_path) == -1){
      fprintf(stderr, "mv : Erreur rencontrée lors de l'opération.\n");
      return -1;
    }else{
      if(mes_flags->flag_v == 1){
        fprintf(stderr, "'%s' -> '%s'\n", src_path, dest_path);
      }
    }
  }
  // Sinon, si le déplacement concerne un fichier -> un dossier ou un dossier -> 1 fichier --> Erreur
  else{
    if(S_ISDIR(dest_stat.st_mode)){
      fprintf(stderr, "mv: impossible d'écraser le répertoire '%s' par le non répertoire '%s'.\n", dest_path, src_path);
    }else{
      fprintf(stderr, "mv: impossible d'écraser le non répertoire '%s' par le répertoire '%s'.\n", dest_path, src_path);
    }
    return -1;
  }
  return 0;
}

/**
 * Indique si un fichier est un dossier ou non
 * @param  chemin le chemin du fichier
 * @return 0 si le fichier n'existe pas, 2 si c'est un dossier, 1 sinon
 */
int est_un_dossier(char *chemin){

  struct stat stat_struct = {0};
  if (stat(chemin, &stat_struct) != 0){
    return 0; // Le fichier n'existe pas
  }
  else if (S_ISDIR(stat_struct.st_mode)){
    return 2; // C'est un dossier
  }else{
    return 1; // C'est un fichier
  }
}


/**
 * Permet de concaténer les chemins du fichier source et du fichier de destination correctement
 * @param  src_path  le chemin du fichier source
 * @param  dest_path le chemin du dossier de destination
 * @return le chemin formé par le chemin source et le chemin de destination
 */
char * creer_chemin_final(char* src_path, char* dest_path){

  char *nom_source;                       // Nom du fichier source (exemple rep/rep1/fic1 --> fic1)
  int borneMinSrc = 0;                    // Borne minimale pour supprimer l'éventuel '/' devant le nom du fichier source
  int nb_slashs_src_fin = 0;              // Pour supprimer les slashs en fin du fichier source
  char *nom_final_fic;                    // Chemin de destination final
  int longueur_totale = 0;                // Longueur totale du chemin de destination final
  int longueur_dest = strlen(dest_path);  // Longueur du chemin destination
  int borneMaxDst = longueur_dest-1;      // Pour supprimer le dernier slash du chemin de destination

  // Supprimer les slashs de fin de fichier source
  int i = strlen(src_path) - 1;
  while(src_path[i] == '/' && i >= 0){
    nb_slashs_src_fin++;
    src_path[i] = ' ';
    i--;
  }

  // Récupération du nom du fichier source (sans le chemin qui le précède)
  if((nom_source = strrchr(src_path, '/')) == NULL){
    nom_source = src_path;
  }else{
    // On supprime l'éventuel slash en début de chemin source
    borneMinSrc++;
    longueur_totale--;
  }

  longueur_totale += strlen(nom_source) + longueur_dest + 1 - nb_slashs_src_fin; // + 1 pour le slash

  // On supprime l'éventuel slash en fin de chemin destination
  if(dest_path[longueur_dest-1] == '/'){
    borneMaxDst--;
    longueur_totale--;
  }

  nom_final_fic = malloc(sizeof(char) * (longueur_totale + 1)); // + 1 pour le caractère de fin de chaîne de caractères
  if(nom_final_fic == NULL){
    fprintf(stderr, "cp: Erreur d'allocation de la mémoire lors de la copie\n");
    return NULL;
  }

  i = 0;
  // Ajout du chemin de destination
  while(i <= borneMaxDst){
    nom_final_fic[i] = dest_path[i];
    i++;
  }

  // Ajout du slash séparant le chemin de destination du nom du fichier source
  nom_final_fic[borneMaxDst+1] = '/';
  i++;

  // Ajout du nom du fichier source
  for (int j = borneMinSrc; i < longueur_totale; j++) {
    nom_final_fic[i] = nom_source[j];
    i++;
  }

  nom_final_fic[longueur_totale] = '\0';

  return nom_final_fic;
}
