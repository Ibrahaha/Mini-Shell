# En étant placé dans le répertoire du projet
# faire : export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(pwd)/lib
# pour que cmdr3 fonctionne

CC = gcc
CCFLAGS = -Wall -pedantic -std=c11 -fPIC
OBJECTS = cp.o ls.o mkdir.o pwd.o cat.o du.o mv.o rm.o chmod.o main.o
CURRENTDIR = $(shell pwd)



all: cmdr cmdr2 cp ls mkdir pwd cat du mv rm cmdr3 su

# Intégré à l'exécutable interpréteur de commande.
cp_1.o: cp.c cp.h Makefile
	$(CC) $(CCFLAGS) -c cp.c

ls_1.o: ls.c ls.h Makefile
	$(CC) $(CCFLAGS) -c ls.c

mkdir_1.o: mkdir.c mkdir.h Makefile
	$(CC) $(CCFLAGS) -c mkdir.c

pwd_1.o: pwd.c pwd.h Makefile
	$(CC) $(CCFLAGS) -c pwd.c

cat_1.o: cat.c cat.h Makefile
	$(CC) $(CCFLAGS) -c cat.c

mv_1.o: mv.c mv.h Makefile
	$(CC) $(CCFLAGS) -c mv.c

du_1.o: du.c du.h Makefile
	$(CC) $(CCFLAGS) -c du.c

rm_1.o: rm.c rm.h Makefile
	$(CC) $(CCFLAGS) -c rm.c

chmod_1.o: chmod.c chmod.h Makefile
	$(CC) $(CCFLAGS) -c chmod.c

main_1.o: main.c main.h cp.h ls.h mkdir.h pwd.h cat.h du.h mv.h rm.h chmod.h Makefile
	$(CC) $(CCFLAGS) -DEXEPATH="\"$(CURRENTDIR)\"" -DTYPECOMPILATION=1 -c main.c

cmdr: cp_1.o ls_1.o mkdir_1.o pwd_1.o cat_1.o du_1.o mv_1.o rm_1.o chmod_1.o main_1.o
	$(CC) $(CCFLAGS) -o cmdr $(OBJECTS) -lm


# Séparé en différents exécutables.
cp_2.o: cp.c cp.h Makefile
	$(CC) $(CCFLAGS) -DEXE -c cp.c

ls_2.o: ls.c ls.h Makefile
	$(CC) $(CCFLAGS) -DEXE -c ls.c

mkdir_2.o: mkdir.c mkdir.h Makefile
	$(CC) $(CCFLAGS) -DEXE -c mkdir.c

pwd_2.o: mkdir.c mkdir.h Makefile
	$(CC) $(CCFLAGS) -DEXE -c pwd.c

cat_2.o: cat.c cat.h Makefile
	$(CC) $(CCFLAGS) -DEXE -c cat.c

mv_2.o: mv.c mv.h Makefile
	$(CC) $(CCFLAGS) -DEXE -c mv.c

du_2.o: du.c du.h Makefile
	$(CC) $(CCFLAGS) -DEXE -c du.c

rm_2.o: rm.c rm.h Makefile
	$(CC) $(CCFLAGS) -DEXE -c rm.c

chmod_2.o: chmod.c chmod.h Makefile
	$(CC) $(CCFLAGS) -DEXE -c chmod.c

cp: cp_2.o
	$(CC) $(CCFLAGS) -o cp cp.o

ls: ls_2.o
	$(CC) $(CCFLAGS) -o ls ls.o

mkdir: mkdir_2.o
	$(CC) $(CCFLAGS) -o mkdir mkdir.o

pwd: pwd_2.o
	$(CC) $(CCFLAGS) -o pwd pwd.o

cat: cat_2.o
	$(CC) $(CCFLAGS) -o cat cat.o

mv: mv_2.o
	$(CC) $(CCFLAGS) -o mv mv.o

du: du_2.o
	$(CC) $(CCFLAGS) -o du du.o

rm: rm_2.o
	$(CC) $(CCFLAGS) -o rm rm.o

chmod: chmod_2.o
	$(CC) $(CCFLAGS) -o chmod chmod.o -lm

main_2.o: main.c main.h Makefile
	$(CC) $(CCFLAGS) -DEXEPATH="\"$(CURRENTDIR)\"" -DTYPECOMPILATION=2 -c main.c

cmdr2: main_2.o
	$(CC) $(CCFLAGS) -o cmdr2 $(OBJECTS) -lm


# Librairie
cp_3.o: cp.c cp.h Makefile
	$(CC) $(CCFLAGS) -c cp.c

ls_3.o: ls.c ls.h Makefile
	$(CC) $(CCFLAGS) -c ls.c

mkdir_3.o: mkdir.c mkdir.h Makefile
	$(CC) $(CCFLAGS) -c mkdir.c

pwd_3.o: pwd.c pwd.h Makefile
	$(CC) $(CCFLAGS) -c pwd.c

cat_3.o: cat.c cat.h Makefile
	$(CC) $(CCFLAGS) -c cat.c

mv_3.o: mv.c mv.h Makefile
	$(CC) $(CCFLAGS) -c mv.c

du_3.o: du.c du.h Makefile
	$(CC) $(CCFLAGS) -c du.c

rm_3.o: rm.c rm.h Makefile
	$(CC) $(CCFLAGS) -c rm.c

chmod_3.o: chmod.c chmod.h Makefile
	$(CC) $(CCFLAGS) -c chmod.c

./lib/libCmds.so: cp_3.o ls_3.o mkdir_3.o pwd_3.o cat_3.o du_3.o mv_3.o rm_3.o chmod_3.o Makefile
	mkdir -p ./lib
	$(CC) $(CCFLAGS) -shared -o ./lib/libCmds.so cp.o ls.o mkdir.o pwd.o cat.o du.o mv.o rm.o chmod.o -lm

main_3.o: main.c main.h Makefile
	$(CC) $(CCFLAGS) -DEXEPATH="\"$(CURRENTDIR)\"" -DTYPECOMPILATION=3 -c main.c

cmdr3: ./lib/libCmds.so  main_3.o
	$(CC) -L./lib $(CCFLAGS) -o cmdr3 main.o -ldl -lCmds

# Commande su (toujours en exécutable)
su.o: su.c su.h Makefile
	$(CC) $(CCFLAGS) -DEXEPATH="\"$(CURRENTDIR)\"" -c su.c
su: su.o
	sudo $(CC) $(CCFLAGS) -o su su.o -lcrypt
	sudo chmod u+s su


clean:
	rm *.o cmdr cmdr2 cmdr3 ls cp mkdir cat du mv pwd rm ./lib/*.so
	rm -r lib
	rm -f su
