#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>

#include "mkdir.h"

// Structure qui va contenir les flags à destination des options
struct SFlags
{
  int flag_m ;    // Option pour donner le mode de création du dossier, prend la valeur du mode indiquée
  int flag_p ;    // Option pour créer les dossiers parents s'ils n'existent pas
  int flag_v ;    // Option pour activer le mode "verbeux"
};


#ifdef EXE

int main(int argc, char *argv[]){

	int statusCode;
	statusCode = mkd(argc, argv);
	return statusCode;
}

#endif

/**
 * Permet de créer un dossier.
 * @param  argc le nombre d'arguments passés à la fonction.
 * @param  argv les différents arguments passés à la fonction.
 * @return 0 si l'opération s'est bien passée, -1 sinon.
 */
int mkd(int argc, char *argv[]){

	mode_t mode_creation_dir = 0777; // Mode par défaut (sera modifié en fonction du umask défini)
  int tmp_mode = -1;

	// Flags à destination des options
  SFlags *mes_flags = malloc(sizeof(SFlags));
  mes_flags->flag_m = 0;
  mes_flags->flag_p = 0;
  mes_flags->flag_v = 0;

	int nb_dossiers = 0; // Nombre de dossiers à créer indiqués

	// On récupère les options de notre commande
	for(int i = 1; i < argc; i++){
		if(argv[i][0] == '-'){
			{
				for (unsigned int j = 1; j < strlen(argv[i]); j++) {
					switch (argv[i][j]) {
						case 'm':
              // La valeur du mode doit se trouver dans l'argument suivant
              if(i + 1 < argc){
                errno = 0;
                tmp_mode = mode_valide(argv[i+1]);
                argv[i+1] = " ";
              }
              if(tmp_mode == -1 || i+1 >= argc){
                fprintf(stderr, "mkdir: l'option -m requiert un argument valide.\n");
                free(mes_flags);
                return -1;
              }
              mode_creation_dir = tmp_mode;
							break;
						case 'p':
							mes_flags->flag_p = 1;
							break;
						case 'v':
							mes_flags->flag_v = 1;
							break;
						default:
							fprintf(stderr,"%s: Option %c invalide.\n", argv[0], argv[i][j]);
              free(mes_flags);
              return -1;
					}
				}
			}
		}else{
			nb_dossiers++;
		}
	}


	// Il faut indiquer au moins un chemin
	if(nb_dossiers == 0){
		free(mes_flags);
		fprintf(stderr, "mkdir: opérande manquant.\n");
		return -1;
	}

	char *path;
	int i = 1;
	int nb_dossiers_crees = 0;

	// On parcours de nouveau les arguments pour créer les dossiers
	while(i < argc && nb_dossiers_crees < nb_dossiers){
		// Si ce n'est pas une option, ce doit être un chemin
		if(argv[i][0] != '-' && strcmp(argv[i], " ") != 0){
			nb_dossiers_crees++;
			path = argv[i];

      // Si on souhaite créer une arborescence si celle-ci n'existe pas
      if(mes_flags->flag_p == 1){
        if(creer_arborescence_dossiers(path, mes_flags, mode_creation_dir) == -1){
          free(mes_flags);
          return -1;
        }
      }
      // Simple création d'un dossier
      else{
        if(creer_dossier(path, mes_flags, mode_creation_dir) == -1){
          free(mes_flags);
          return -1;
        }
      }

		}
		i++;
	}
	free(mes_flags);
 	return 0;
}

/**
 * Vérifie qu'une chaîne de caractères correspond bien à un mode possible de création de dossier (forme octale)
 * @param  mode_texte la chaîne de caracteres correspondant au mode souhaité
 * @return la valeur du mode en entier si la chaîne de caracteres correspond bien à un mode possible, -1 sinon
 */
int mode_valide(char* mode_texte){

  int longueur = strlen(mode_texte);
  unsigned int mode = 0;

  for (int i = 0; i < longueur; i++) {
    // On vérifie que la chaîne ne contient que des chiffres entre 0 et 7
    if(mode_texte[i] < '0' || mode_texte[i] > '7'){
      return -1;
    }
  }

  errno = 0;
  mode = strtol(mode_texte, NULL, 8);

  if(errno == EINVAL || errno == ERANGE || mode > 777){
    mode = -1;
  }

  return mode;
}

/**
 * Fonction permettant de créer un dossier au chemin indiqué selon le mode souhaité
 * @param  path              le chemin du dossier à créer
 * @param  mes_flags         les flags qui correspondent aux différentes options de la commande
 * @param  mode_creation_dir la valeur du mode de création du répertoire
 * @return 0 si la création a bien été effectuée, -1 sinon
 */
int creer_dossier(char* path, SFlags *mes_flags, int mode_creation_dir){
  struct stat st = {0};
  // On vérifie qu'un fichier ne porte pas le même nom dans le répertoire de destination
  if (stat(path, &st) == -1) {
    errno = 0;
    // Création du dossier
    if(mkdir(path, mode_creation_dir) != 0){
        printf("mkdir : erreur lors de la création du répertoire %s : %s \n", path, strerror(errno));
        return -1;
    }else{
      // Mode verbeux -> on affiche le nom du répertoire créé
      if(mes_flags->flag_v == 1){
        fprintf(stderr, "mkdir: création du répertoire '%s'.\n", path);
      }
    }
  }
  else{
    if(mes_flags->flag_p == 1){
      return 0;
    }
    fprintf(stderr, "mkdir: impossible de créer le répertoire '%s': Le fichier existe.\n", path);
    return -1;
  }
  return 0;
}

/**
 * Créer une arborescence de dossiers si elle n'existe pas encore
 * @param  path              le chemin du dossier à créer
 * @param  mes_flags         les flags qui correspondent aux différentes options de la commande
 * @param  mode_creation_dir la valeur du mode de création du répertoire
 * @return 0 si la création a bien été effectuée, -1 sinon
 */
int creer_arborescence_dossiers(char* path, SFlags *mes_flags, int mode_creation_dir){

  int i = 0;
  int longueur_path = strlen(path);

  char* current_path = malloc(sizeof(char) * (longueur_path + 1));

  // On compte le nombre de slashs en début de chemin
  while(i < longueur_path && path[i] == '/'){
    i++;
  }

  // S'il y en a au moins un, on va le prendre en compte (chemin par rapport à la racine)
  if(i > 0){
    i--;
  }

  // On enlève les éventuels slashs en fin de chemin
  for(int k = longueur_path-1; k >= 0; k--){
    if(path[k] == '/'){
      path[k] = '\0';
      longueur_path--;
    }else{
      break;
    }
  }

  int j = i;
  // Parcours du chemin et création des éventuels dossiers parents
  while(j < longueur_path){
    current_path[j-i] = path[j];
    if(path[j] == '/' && j > 0 && path[j-1] != '/'){
      current_path[j-i] = '\0';
      if(creer_dossier(current_path, mes_flags, mode_creation_dir) == -1){
        free(current_path);
        return -1;
      }
      current_path[j-i] = '/';
    }
    j++;
  }
  current_path[j-i] = '\0';
  mes_flags->flag_p = 0;

  // Création du dossier final
  if(creer_dossier(current_path, mes_flags, mode_creation_dir) == -1){
    free(current_path);
    return -1;
  }
  mes_flags->flag_p = 1;
  free(current_path);
  return 0;
}
