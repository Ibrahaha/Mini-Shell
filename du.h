#ifndef _DU_H_
#define _DU_H_

	#ifndef SFLAGS
	#define SFLAGS
		typedef struct SFlags SFlags;
	#endif
	typedef struct stat Stat;

	int du(int argc, char *argv[]);

	int get_size(char *current_path, SFlags *mes_flags);
	int get_size_dossier(char *current_path, Stat *stat_cur, SFlags *mes_flags);
	int get_size_fichier(char *current_path, Stat *stat_cur, SFlags *mes_flags);


#endif
