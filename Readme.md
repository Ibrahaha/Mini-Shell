Projet Polybash
============
Réalisation d'un **Mini-Shell** avec le langage C avec l'implémentation de certaines commandes.

Commandes disponibles
============
	- cp
	- ls
	- exit
	- cd
	- pwd
	- mkdir
	- cat
	- su
	- whoami
	- du
	- echo
	- rm
	- chmod
	- commands
	- connect
	- disconnect

Fonctionnalité disponibles
============
	- Exécution de programme utilisateur
	- Multi-pipe
	- Redirection de la sortie standard vers un fichier (> >>)
	- Connexion à un autre terminal (connect addr port)

L'équipe
============
GUITTON Julien
SOULEYMANE Ibrahim
COUGNAUD Julien
MOITEAUX Quentin

Lancement
============
Pour compiler le projet, il faut au préalable se placer dans le répertoire du projet et faire _make_.

3 exécutables sont générés :

	-  cmdr : correspond au polybash avec les commandes intégrées.
	-  cmdr2 : correspond au polybash avec les commandes en exécutable.
	-  cmdr3 : correspond au polybash avec les commandes en librairie.

Pour que _cmdr3_ fonctionne, il est nécessaire de faire **export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(pwd)/lib** en étant dans le répertoire du projet.
