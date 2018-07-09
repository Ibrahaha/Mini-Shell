#ifndef _CP_H_
#define _CP_H_

	char * creer_nom_final(char* src_path, char* dest_path);
	int copier_fichier(char* src_path, char* dest_path, int nature_dest, int iflag, int vflag);
	int copier_dossier(char* src_path, char* dest_path, int nature_dest, int iflag, int vflag);
	int est_dossier(char *chemin);

	int cp(int argc, char *argv[]);

#endif

