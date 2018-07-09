#ifndef _CHMOD_H_
#define _CHMOD_H_


	typedef struct Flags
	{
		int f_flag;     		// Option pour activer le mode force -f
		int v_flag;     		// Option pour activer le mode verbeux -v
		int r_flag;     		// Option pour modifier récursivement les autorisations des réper­toires et de leurs contenus -r/-R.
	} Flags;

	int chmod1(int argc, char*argv[]);
	int conversion_decimal(int n);
	void chmod0(char *chemin, int options, int permissions);
	void modif_perm(char *chemin, int options, int permissions, Flags *flags);
	void parcours(char *chemin, int options, int permissions, Flags *flags);

#endif
