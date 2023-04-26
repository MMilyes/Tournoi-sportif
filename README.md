# Tournoi-sportif
simulation d'un tournoi sportif en C -multithreading-

Notre programme de simulation de tournoi sportif simule des matchs de différentes équipes dans différents sports.
Les noms des équipes sont stockés dans un fichier texte et les scores sont générés aléatoirement.
Les résultats sont enregistrés dans un fichier de sortie et le logiciel permet une synchronisation pour que les matchs ne se chevauchent
pas.

Le but de ce projet est la maitrise des principes fondamentaux de programmation système / processus et communications ...etc.

# Paramétrage
Plusieurs options sont disponibles pour paramétrer votre lancement :

Tout d’abord vous avez la possibilité de fournir un fichier texte personnalisé contenant des équipes de votre choix sous ce format : "pays 1, pays 2, pays 3 ", un fichier par défaut
"equipes_default.txt" sera utilisé si vous n’avez rien fourni.

Le fichier options.txt contient 2 valeurs numériques que vous pouvez modifier :

-La première est ‘temps_avant_le_lancement_d'un_tour’ (en secondes).

-La deuxième est ‘vitesse’ (en millisecondes) celle-ci représente le temps entre chaque
round d’un même match.

# Prise en main
1. Placez-vous dans le répertoire du programme et ouvrez une console de commande.
2. Exécutez la commande ``` make ``` pour compiler le programme, puis exécutez la commande ``` "./OS" ``` pour lancer le programme.
3. Si vous avez votre propre fichier texte à fournir contenant les équipes vous devez exécuté le programme comme ceci : ``` ./OS nom_fichier.txt ```, assurer-vous que votre fichier répond à la forme demandé et qu’il contienne un nombre d’équipe d’une
puissance de 2. Sinon le fichier par défaut sera exécuté.
4. A la fin de l’exécution vous pouvez voir tout le déroulement des matchs dans un fichier texte nommé "deroulement.txt" et se trouvera dans le répertoire courant.
5. Un dossier "doc" contenant la documentation doxygen sera disponible après l’exécution de la commande ``` make ```, si cela ne sait pas fait vous devez installer doxygen sur votre machine.
6. Une alternative à la commande ``` make ``` est ``` gcc OS.c -o OS – lpthread ``` pour compiler le programme, puis : ``` ./OS ```

# Informations complémentaires
Le programme est doté d’un système de gestion de problèmes et des erreurs dont :
1. Fichier texte erroné.
2. Les équipes fournies ne sont pas une puissance de 2.
3. Prise du fichier par défaut en cas de problèmes.
4. Création/suppression de fichier existant.
5. Fichier texte vide.




