#ifndef _MKDIR_H_
#define _MKDIR_H_

	#ifndef SFLAGS
	#define SFLAGS
		typedef struct SFlags SFlags;
	#endif

	int creer_dossier(char* path, SFlags *mes_flags, int mode_creation_dir);
	int creer_arborescence_dossiers(char* path, SFlags *mes_flags, int mode_creation_dir);
	int mode_valide(char* mode_texte);
	int mkd(int argc, char *argv[]);

#endif
