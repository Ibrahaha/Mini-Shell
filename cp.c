#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <dirent.h>
#include "cp.h"

#ifdef EXE

int main(int argc, char *argv[]){
  int statusCode;
  statusCode = cp(argc, argv);
  return statusCode;
}

#endif

/**
 * Permet de savoir si un fichier est un dossier ou non
 * @param   chemin le chemin du fichier
 * @return  0 si le fichier n'existe pas, 2 si c'est un dossier, 1 sinon
 */
int est_dossier(char *chemin){

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
 * Permet de concaténer correctement un chemin source et un chemin destination
 * @param  src_path  le chemin du fichier source
 * @param  dest_path le chemin du dossier de destination
 * @return l'association correcte des deux chemins
 */
char * creer_nom_final(char* src_path, char* dest_path){

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

/**
 * Copie un fichier vers le répertoire indiqué ou courant.
 * @param  argc le nombre d'arguments passés à la fonction.
 * @param  argv les différents arguments passés à la fonction.
 * @return 0 si l'opération s'est bien passée, -1 sinon.
 */
int cp(int argc, char *argv[]){

  int iflag  = 0;     		// Option pour demander confirmation avant chaque suppression
  int vflag  = 0;     		// Option pour activer le mode verbeux
  int rflag  = 0;     		// Option pour réaliser les copies de manière récursive (pour la copie des dossiers)

  int nb_fichiers = 0; 		// Nombre de chemins de fichiers / dossiers en arguments de la commande
  int nature_dest = -1; 	// Indique la nature de la destination : -1 (non définie), 0 (n'existe pas), 1 (non dossier), 2 (dossier)
  char *src_path;					// Chemin source
  char *dest_path;				// Chemin de destination

  // Il faut au moins 2 arguments
  if(argc < 3){
    fprintf(stderr, "cp : opérande de fichier manquant.\n");
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
            iflag = 1;
            break;
            case 'v':
            vflag = 1;
            break;
            case 'r':
            rflag = 1;
            break;
            default:
            fprintf(stderr, "cp: Option %c invalide.\n", argv[i][j]);
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
        // On regarde si cela correspond à un dossier
        nature_dest = est_dossier(dest_path);
      }
      nb_fichiers++;
    }
  }

  // Il faut indiquer au moins deux chemins pour exécuter la commande
  if(nb_fichiers < 2){
    fprintf(stderr, "cp : opérande de fichier manquant.\n");
    return -1;
  }

  int nb_fichiers_traites = 1;	// Nombre de fichiers (ou dossiers) traités, 1 par défaut (le répertoire de destination)
  int nature_source = -1;       // Nature du fichier source
  int i = 1;

  // Cas où les deux arguments sont soit : deux fichiers, deux dossiers ou bien un fichier et un dossier
  if(nb_fichiers == 2){

    // Récupération du chemin source
    while(i < argc){
      if(argv[i][0] != '-'){
        src_path = argv[i];
        break;
      }
      i++;
    }

    nature_source = est_dossier(src_path); // 0 (n'existe pas), 1 (pas dossier) ou 2 (dossier)

    // Le fichier source n'existe pas --> Erreur
    if(nature_source == 0){
      fprintf(stderr, "cp : impossible d'évaluer '%s': Aucun fichier ou dossier de ce type\n", src_path);
      return -1;
    }

    // Le fichier source n'est pas un dossier
    if(nature_source == 1){
      return copier_fichier(src_path, dest_path, nature_dest, iflag, vflag);
    }

    // Le fichier source est un dossier
    if(nature_source == 2){
      if(rflag == 0){
        fprintf(stderr, "cp : omission du répertoire '%s'.\n", src_path);
        return -1;
      }else{
        return copier_dossier(src_path, dest_path, nature_dest, iflag, vflag);
      }
    }

  }

  // Copier en même temps plusieurs fichiers (et/ou dossiers) dans un répertoire
  else{

    // Le dossier de destination n'existe pas --> Opération impossible
    if(nature_dest != 2){
      fprintf(stderr, "cp : la cible '%s' n'est pas un répertoire.\n", dest_path);
      return -1;
    }

    // Le dossier de destination existe, on va parcourir les différents arguments pour effectuer les copies
    i = 1;
    while(i < argc && nb_fichiers_traites < nb_fichiers){
      // Si ce n'est pas une option, ce doit être un chemin
      if(argv[i][0] != '-'){
        nb_fichiers_traites++;
        src_path = argv[i];
        nature_source = est_dossier(src_path);
        // La source n'existe pas --> Erreur et on passe au suivant
        if(nature_source == 0){
          fprintf(stderr, "cp : impossible d'évaluer '%s': Aucun fichier ou dossier de ce type\n", src_path);
          continue;
        }

        // Le fichier source n'est pas un dossier
        if(nature_source == 1){
          copier_fichier(src_path, dest_path, nature_dest, iflag, vflag);
        }

        // Le fichier source est un dossier, on vérifie que l'option r est bien présente
        if(nature_source == 2){
          if(rflag == 0){
            fprintf(stderr, "cp : omission du répertoire '%s'.\n", src_path);
            continue;
          }else{
            copier_dossier(src_path, dest_path, nature_dest, iflag, vflag);
          }
        }
      }
      i++;
    }
  }
  return 0;
}


/**
 * Permet de copier un fichier depuis un chemin source vers un chemin de destination
 * @param  src_path    le chemin du fichier soure
 * @param  dest_path   le chemin du fichier de destination
 * @param  nature_dest la nature du fichier de destination (dossier : 2, sinon : 1)
 * @param  iflag       le flag caractérisant l'option i
 * @param  vflag       le flag caractérisant l'option v
 * @return 0 si la copie a bien été effectuée, -1 sinon
 */
int copier_fichier(char* src_path, char* dest_path, int nature_dest, int iflag, int vflag){

  int source;           // Fichier source
  int destination;      // Fichier destination
  char* nom_final_dest; // Chemin de destination finale

  // On vérifie que les deux chemins ne pointent pas vers le même fichier
  struct stat src, dst;

  if (stat(src_path, &src) == 0 && stat(dest_path, &dst) == 0) {

    if ((src.st_dev == dst.st_dev) && (src.st_ino == dst.st_ino)) {
      fprintf(stderr, "cp: Impossible de copier le fichier %s sur lui-même (%s).\n", src_path, dest_path);
      return -1;
    }

  }

  // Ouverture du fichier source
  if((source=open(src_path, O_RDONLY)) < 0){
    fprintf(stderr, "cp : Erreur lors de l'ouverture du fichier %s.\n", dest_path);
    return -1;
  }

  // Ouverture du fichier destination qui existe
  if(nature_dest == 1){
    // Si l'option i est activée --> demande à l'utilisateur s'il souhaite écraser le fichier destination
    if(iflag == 1){
      fprintf(stderr, "cp : voulez-vous écraser '%s' ? (y/n)\n", dest_path);
      char reponse = getchar();
      // si l'utilisateur saisit autre chose que 'y', la copie n'est pas effectuée
      if (reponse != 'y'){
        return 0;
      }
    }

    if((destination=open(dest_path, O_WRONLY | O_TRUNC)) < 0){
      fprintf(stderr, "cp : Erreur lors de l'ouverture du fichier %s.\n", dest_path);
      return -1;
    }
    nom_final_dest = malloc(sizeof(char)*(1+strlen(dest_path)));
    nom_final_dest = strcpy(nom_final_dest, dest_path);

  }

  // La destination est un dossier qui existe, on va copier le fichier dedans sans changer de nom
  else if(nature_dest == 2){

    nom_final_dest = creer_nom_final(src_path, dest_path);
    int type_fic = est_dossier(nom_final_dest);

    // Si un dossier dans le répertoire cible porte déjà le nom du fichier -> opération impossible
    if(type_fic == 2){
      fprintf(stderr, "cp : impossible d'écraser le répertoire '%s' par un non répertoire.\n", nom_final_dest);
      free(nom_final_dest);
      return -1;
    }

    // Si l'option i est activée
    if(iflag == 1){
      // Si le fichier de destination existe, on va demander si l'utilisateur veut l'écraser ou non
      if(type_fic == 1){
        fprintf(stderr, "cp : voulez-vous écraser '%s' ? (y/n)\n", nom_final_dest);
        char reponse = getchar();
        // si l'utilisateur saisit autre chose que 'y', la copie n'est pas effectuée
        if (reponse != 'y'){
          free(nom_final_dest);
          return 0;
        }
      }
    }
    destination = open(nom_final_dest, O_WRONLY | O_CREAT, 0664);
    if(destination < 0){
      fprintf(stderr, "cp : Erreur lors de l'ouverture du fichier %s.\n", dest_path);
      free(nom_final_dest);
      return -1;
    }
  }

  // C'est un fichier ou un dossier qui n'existe pas
  else{
    // C'est un dossier --> erreur
    if(dest_path[strlen(dest_path)-1] == '/'){
      fprintf(stderr, "cp: impossible d'accéder à '%s': N'est pas un dossier.\n", dest_path);
      return -1;
    }
    // C'est un fichier --> on va le créer dans le répertoire courant
    else{
      if((destination=open(dest_path, O_WRONLY | O_CREAT, 0664)) < 0){
        fprintf(stderr, "cp : Erreur lors de l'ouverture du fichier %s.\n", dest_path);
        return -1;
      }
      nom_final_dest = malloc(sizeof(char)*(1+strlen(dest_path)));
      nom_final_dest = strcpy(nom_final_dest, dest_path);

    }
  }
  char c;
  int n_read;
  int n_write;

  // On vérifie que l'on ne copie pas un fichier vers lui-même (cas du répertoire courant par exemple "cp fic .")
  struct stat dst_final;

  if (stat(nom_final_dest, &dst_final) == 0) {

    if ((src.st_dev == dst_final.st_dev) && (src.st_ino == dst_final.st_ino)) {
      fprintf(stderr, "cp: Impossible de copier le fichier %s sur lui-même (%s).\n", src_path, nom_final_dest);
      free(nom_final_dest);
      return -1;
    }

  }

  // Lecture du fichier source
  while((n_read=read(source, &c, 1)) > 0){
    // Ecriture dans le fichier destination
    if((n_write=write(destination, &c, 1)) < 0){
      fprintf(stderr, "cp : Erreur lors de l'écriture dans le fichier '%s'.\n", dest_path);
      free(nom_final_dest);
      return -1;
    }else if(n_write != n_read){
      break;
    }
  }
  // Si le mode verbeux est activé, on précise la copie effectuée.
  if(vflag == 1){
    fprintf(stderr, "'%s' --> '%s'\n", src_path, nom_final_dest);
  }
  free(nom_final_dest);


  // Fermeture des fichiers source et destination
  if (close(source) < 0) {
    fprintf(stderr, "cp : Erreur lors de la fermeture du fichier %s.\n", src_path);
    return -1;
  }
  if (close(destination) < 0) {
    fprintf(stderr, "cp : Erreur lors de la fermeture du fichier %s.\n", dest_path);
    return -1;
  }
  return 0;
}

/**
 * Permet de copier un dossier depuis un chemin source vers un chemin de destination
 * @param  src_path    le chemin du fichier soure
 * @param  dest_path   le chemin du fichier de destination
 * @param  nature_dest la nature du fichier de destination (dossier : 2, sinon : 1)
 * @param  iflag       le flag caractérisant l'option i
 * @param  vflag       le flag caractérisant l'option v
 * @return 0 si la copie a bien été effectuée, -1 sinon
 */
int copier_dossier(char* src_path, char* dest_path, int nature_dest, int iflag, int vflag){
  char *chemin_nv_dossier; // Chemin du dossier final

  // La destination est un fichier --> erreur
  if(nature_dest == 1){
    fprintf(stderr, "cp : impossible d'écraser le non répertoire '%s' par le répertoire '%s'.\n", dest_path, src_path);
    return -1;
  }

  // On vérifie que les deux chemins ne pointent pas vers le même dossier
  struct stat src, dst;

  if (stat(src_path, &src) == 0 && stat(dest_path, &dst) == 0) {

    if ((src.st_dev == dst.st_dev) && (src.st_ino == dst.st_ino)) {
      fprintf(stderr, "cp: Impossible de copier le fichier %s sur lui-même (%s).\n", src_path, dest_path);
      return -1;
    }

  }

  // La destination est un dossier existant, on va copier notre dossier dedans
  if(nature_dest == 2){
    chemin_nv_dossier = creer_nom_final(src_path, dest_path);
  }
  // La destination n'est pas existante
  else{
    chemin_nv_dossier = malloc(sizeof(char)*(1+strlen(dest_path)));
    chemin_nv_dossier = strcpy(chemin_nv_dossier, dest_path);
  }

  // On vérifie que l'on ne copie pas un dossier vers lui-même (cas répertoire courant)
  struct stat dst_final;

  if (stat(chemin_nv_dossier, &dst_final) == 0) {

    if ((src.st_dev == dst_final.st_dev) && (src.st_ino == dst_final.st_ino)) {
      fprintf(stderr, "cp: Impossible de copier le dossier %s sur lui-même (%s).\n", src_path, chemin_nv_dossier);
      free(chemin_nv_dossier);
      return -1;
    }

  }

  // On tente de créer le dossier
  if(mkdir(chemin_nv_dossier, 0775) != 0){
    fprintf(stderr, "cp: impossible de créer le répertoire '%s'.\n", chemin_nv_dossier);
    free(chemin_nv_dossier);
    return -1;
  }

  DIR *dirp;
  struct dirent *dptr;

  char* chemin_fic_courant;
  char* chemin_fic_dest_courant;

  // Ouverture du dossier source en vue de la copie de son contenu
  if((dirp = opendir(src_path)) == NULL){
    fprintf(stderr, "cp: impossible d'ouvrir le dossier '%s'.\n", src_path);
    free(chemin_nv_dossier);
    return -1;
  }

  // Si le mode verbeux est activé, on précise que le dossier a été créé
  if(vflag == 1){
    fprintf(stderr, "'%s' --> '%s'\n", src_path, chemin_nv_dossier);
  }

  while((dptr = readdir(dirp)) != NULL){
    // Si le fichier courant ne correspond ni au dossier courant, ni au parent
    if(strcmp(dptr->d_name, ".") != 0 && strcmp(dptr->d_name, "..") != 0){

      chemin_fic_courant = creer_nom_final(dptr->d_name, src_path); // Reconstitution du chemin du fichier source
      chemin_fic_dest_courant = creer_nom_final(chemin_fic_courant, chemin_nv_dossier); // Constitution du chemin de destination

      // Si c'est un répertoire existant
      if(est_dossier(chemin_fic_courant) == 2){
        copier_dossier(chemin_fic_courant, chemin_fic_dest_courant, est_dossier(chemin_fic_dest_courant), iflag, vflag);
      }
      // Sinon c'est un fichier
      else{
        copier_fichier(chemin_fic_courant, chemin_fic_dest_courant, est_dossier(chemin_fic_dest_courant), iflag, vflag);
      }

      free(chemin_fic_courant);
      free(chemin_fic_dest_courant);
    }

  }
  free(chemin_nv_dossier);

  closedir(dirp);
  return 0;

}
