#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "echo.h"

#ifdef EXE

int main(int argc, char *argv[]){

  int statusCode;
  statusCode = echo(argc, argv);
  return statusCode;
}

#endif


/**
 * Permet d'afficher un message saisi par l'utilisateur avec ou sans un saut de ligne à la fin.
 * @param  argc le nombre d'arguments passés à la fonction.
 * @param  argv les différents arguments passés à la fonction.
 * @return 0 si l'opération s'est bien passée, -1 sinon.
 */
int echo(int argc, char *argv[]){

  int nflag  = 0;     // Option pour ne pas effectuer le saut de ligne après le dernier message
  char espace = ' ';

  // On parcours les différents arguments
  for(int i = 1; i < argc; i++){
    // Si on rencontre l'option -n -> on active le flag correspondant
    if(strlen(argv[i]) == 2 && argv[i][0] == '-' && argv[i][1] == 'n'){
      nflag = 1;
    }else{
      if(write (STDOUT_FILENO, argv[i], strlen(argv[i])) < 0){
        fprintf(stderr, "%s : Erreur lors de l'écriture sur la sortie standard.\n", argv[0]);
      }
      // On rajoute un espace entre les différents messages
      if(i < argc - 1){
        if(write (STDOUT_FILENO, &espace, 1) < 0){
          fprintf(stderr, "%s : Erreur lors de l'écriture sur la sortie standard.\n", argv[0]);
        }
      }
    }
  }

  // Si l'option n est activée -> on ne rajoute pas de saut à la ligne
  if(nflag == 0){
    write(STDOUT_FILENO, "\n", strlen("\n"));
  }

  return 0;
}
