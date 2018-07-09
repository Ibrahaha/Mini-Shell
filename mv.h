#ifndef _MV_H_
#define _MV_H_


	#ifndef SFLAGS
	#define SFLAGS
		typedef struct SFlags SFlags;
	#endif

	char * creer_chemin_final(char* src_path, char* dest_path);
	int est_un_dossier(char *chemin);

	int deplacement_vers_dossier(char* src_path, char* dest_path, SFlags *flags);
	int deplacement_cible_existante(char* src_path, char* dest_path, SFlags *flags);

	int mv(int argc, char *argv[]);


#endif
