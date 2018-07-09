#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include "cat.h"

#ifdef EXE

int main(int argc, char *argv[]){

  int statusCode;
  statusCode = cat(argc, argv);
  return statusCode;
}

#endif


/**
 * Concatène des fichers et les affichent sur la sortie standard.
 * @param  argc le nombre d'arguments passés à la fonction.
 * @param  argv les différents arguments passés à la fonction.
 * @return 0 si l'opération s'est bien passée, -1 sinon.
 */
int cat(int argc, char *argv[]){

  int nflag  = 0;     // Option pour afficher le numéro des lignes
  int eflag  = 0;     // Option pour afficher un '$' en fin de ligne

  int nb_fichiers = 0;

  // On récupère les options de notre commande
  for(int i = 1; i < argc; i++){
    if(argv[i][0] == '-'){
      {
        for (unsigned int j = 1; j < strlen(argv[i]); j++) {
          switch (argv[i][j]) {
            case 'E':
              eflag = 1;
              break;
            case 'n':
              nflag = 1;
              break;
            default:
              fprintf(stderr, "%s: Option %c invalide.\n", argv[0], argv[i][j]);
              return -1;
          }
        }
      }
    }else{
      nb_fichiers++;
    }
  }

  int file;                   // Fichier courant
  char caractere;             // Caractère courant
  char prev_caractere = '\n'; // Caractère précédent
  char dollar = '$';          // Pour option -E
  int line = 0;               // Numéro de ligne
  int i = 1;

  // On parcours de nouveau les arguments pour concaténer les fichiers et / ou l'entrée standard
  while(i < argc || nb_fichiers == 0){

    // Cas où aucun fichier n'est passé en paramètre (ou option '-') --> lecture de l'entrée standard
    if(nb_fichiers == 0 || strcmp(argv[i], "-") == 0){

      while(read(STDIN_FILENO, &caractere, 1) > 0){

        // Affichage des numéros de ligne
        if( nflag == 1 && prev_caractere == '\n'){

          line++;
          print_nb_lignes(argv[0], line);

        }

        // Affichage d'un '$' en fin de ligne
        if(eflag == 1 && caractere == '\n'){
          if(write (STDOUT_FILENO, &dollar, 1) < 0){
            fprintf(stderr, "%s : Erreur lors de l'écriture sur la sortie standard.\n", argv[0]);
          }
        }

        // Affichage du caractère courant
        if(write(STDOUT_FILENO, &caractere, 1) < 0){
          fprintf(stderr, "%s : Erreur lors de l'écriture sur la sortie standard.\n", argv[0]);
        }
        prev_caractere = caractere;
      }
      if(nb_fichiers == 0){
        return 0;
      }
    }

    // Sinon, on va concaténer les fichiers dont les chemins sont passés en arguments
    else if(argv[i][0] != '-'){

      char *chemin_fichier = (char*) argv[i];

      // Ouverture du fichier
      if ((file=open(chemin_fichier, O_RDONLY, 0)) < 0) {
        fprintf(stderr, "%s : Erreur lors de l'ouverture du fichier %s.\n", argv[0], chemin_fichier);
      }else{

        // On vérifie que c'est bien un fichier
        struct stat stat_struct = {0};
        if (stat(chemin_fichier, &stat_struct) != 0){
          fprintf(stderr, "%s : Fichier %s non accessible.\n", argv[0], chemin_fichier);
        }
        if (S_ISDIR(stat_struct.st_mode)){
          fprintf(stderr, "%s : %s est un dossier.\n", argv[0], chemin_fichier);
        }else{
          int n = 0;
          // Lecture du fichier
          while ((n=read(file, &caractere, 1)) == 1){
            // Affichage du numéro de ligne pour la 1ère ligne
            if(line == 0 && nflag == 1){
              line++;
              print_nb_lignes(argv[0], line);
            }

            // Affichage d'un '$' en fin de ligne
            if(eflag == 1 && caractere == '\n'){
              if(write (STDOUT_FILENO, &dollar, 1) < 0){
               fprintf(stderr, "%s : Erreur lors de l'écriture sur la sortie standard.\n", argv[0]);
              }
            }

            // Affichage du caractère courant
            if(write(STDOUT_FILENO, &caractere, 1) < 0){
              fprintf(stderr, "%s : Erreur lors de l'écriture sur la sortie standard.\n", argv[0]);
            }

            // Affichage des numéros de ligne
            if(nflag == 1 && caractere == '\n'){
              line++;
              print_nb_lignes(argv[0], line);
            }
          }

          // Fermeture du fichier
          if (close(file) < 0) {
            fprintf(stderr, "%s : Erreur lors de la fermeture du fichier %s.\n", argv[0], chemin_fichier);
          }
        }
      }
    }
    i++;
  }
    write(STDOUT_FILENO, "\n", 1);
    return 0;
}

/**
 * Affichage du nombre de lignes courant en sortie standard
 * @param name_cmd le nom de la commande exécutée
 * @param nb_lines le nombre de lignes ayant déja été affichées
 */
void print_nb_lignes(char *name_cmd, int nb_lines){
    char espace[3] = "   ";

    int length = snprintf( NULL, 0, "%d", nb_lines );
    char* str = malloc( length + 1 );
    snprintf( str, length + 1, "%d", nb_lines );

    if(write (STDOUT_FILENO, &espace, sizeof str) < 0){
        fprintf(stderr, "%s : Erreur lors de l'écriture sur la sortie standard.\n", name_cmd);
    }
    if(write (STDOUT_FILENO, str, sizeof str) < 0){
        fprintf(stderr, "%s : Erreur lors de l'écriture sur la sortie standard.\n", name_cmd);
    }
    if(write (STDOUT_FILENO, &espace, sizeof str) < 0){
        fprintf(stderr, "%s : Erreur lors de l'écriture sur la sortie standard.\n", name_cmd);
    }

    free(str);
}


