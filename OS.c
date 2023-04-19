/**
 * @file OS.c
 * @brief Programme pour simuler un tournoi sportif.
 * @author Mohammed ilyes MALKI
 * @author Djouhoudi SOILIHI
 * Le programme simule un tournoi sportif avec des équipes et des matchs.
 * Les équipes sont créées à partir d'un fichier de configuration et les matchs
 * sont joués en utilisant des threads.
 */
/**
 *  \mainpage Projet system exploitation
 *
 * \section  Introduction
 *
 * Notre programme de simulation de tournoi sportif simule des matchs de différentes
 * équipes dans différents sports.
 * Les noms des équipes sont stockés dans un fichier texte et
 * les scores sont générés aléatoirement.
 * Les résultats sont enregistrés dans un fichier de
 * sortie et le logiciel permet une synchronisation pour que les matchs ne se chevauchent
 * pas. Le but de ce projet est la maitrise des principes fondamentaux de programmation
 * système / processus et communications ...etc
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>
#include <semaphore.h>

#define TAILLEMAXNOM 50
#define TAILLEMAXLIGNE 5000
volatile int threadCount = 0;


typedef struct{
    char nom[TAILLEMAXNOM];
    int resultat;
} Equipe;

typedef struct{// * parametre du thread
    char* half; // * le nom de la moitié du tableau de match
    int taille; // * la taille du tableau de match
    int temps; // * le temps du match
    Equipe *equipe; // * le tableau d'équipes
    int speed; // * la vitesse de jeu du match
} ParThread;

typedef struct{
    int taille; // * la taille du tableau d'équipes
    Equipe *equipe; // * le tableau d'équipes
} nombreEquipe;

typedef struct { // *parametre du match
    char* half; // * la moitié du tableau de match dans lequel le match est joué
    int cote_equipe; 
    Equipe *cote; // * le tableau d'équipes
    int nbrCote; 
    int a; // * le numéro du match
    pthread_barrier_t *barrier; // * la barrière de synchronisation
    pthread_barrier_t *end_barrier; // * la barrière de fin de match
    int speedy; // * la vitesse de jeu du match
} Mpar;

/**
 * @brief Initialise une équipe avec un nom donné.
 * @param nom Le nom de l'équipe.
 * @return L'équipe initialisée.
 */
Equipe initEquipe(char *nom){
    Equipe equ;
    strcpy(equ.nom, nom);
    equ.resultat = 0;
    return equ;
}

/**
 * @brief Ajoute un point aléatoire à une équipe.
 * @return Le nombre de points ajouté (0 ou 1).
 * 
 * Cette fonction utilise la fonction `gettimeofday` pour obtenir un temps aléatoire
 * et l'utilise pour initialiser le générateur de nombres aléatoires.
 * Ensuite, elle utilise la fonction `rand` pour générer un nombre aléatoire entre 0 et 1,
 * qui représente le nombre de points ajouté à l'équipe.
 */
int ajoutdepoint() {
    struct timeval tm;
    gettimeofday(&tm, NULL);
    srandom(tm.tv_sec + tm.tv_usec * 1000000ul); // * initialise le générateur de nombres aléatoires
    return  rand() % 2; // * retourne un nombre aléatoire entre 0 et 1
}

/**
*@brief Joue un match entre deux équipes et renvoie l'équipe gagnante.
*@param equi1 Pointeur vers la première équipe.
*@param equi2 Pointeur vers la deuxième équipe.
*@param a Chaîne de caractères indiquant le temps de jeu (1ere moitié ou 2eme moitié).
*@param s Délai en microsecondes entre chaque point marqué.
*@return L'équipe gagnante.
*/
Equipe matchexec(Equipe *equi1, Equipe *equi2, char* a, int s){
    char* moitie;
    if (strcmp(a,"right")){
        moitie = "1ere moitié";} else {moitie = "2eme moitié";}
    
    int pipefd[2];/* Création d'un tube pour la communication entre les processus. */
    if (pipe(pipefd) == -1){
        perror("pipe");
        exit(1);
    }
    /* Initialisation des scores et des variables de tour. */
    int s_equip1 = 0; int s_equip2 = 0; int ancien1 = 0; int ancien2 = 0;
    int pid = fork(); /* Création d'un nouveau processus pour jouer le match. */

    if (pid == 0){
        FILE *fp; /* Ouverture du fichier de déroulement du match. */
        fp = fopen("deroulement.txt", "a");
        if (fp == NULL){
            perror("fopen");
            exit(1);
        }
        if(strcmp(a,"F")){  /* Ecriture de l'en-tête du match dans le fichier de déroulement. */
        fprintf(fp, " Le matche est entre  %s et %s (%s) \n", equi1->nom, equi2->nom, moitie);
        printf(" Le matche est entre  %s et %s (%s) \n", equi1->nom, equi2->nom, moitie);
        }
        else{
            fprintf(fp, " Les finalistes sont %s et %s \n", equi1->nom, equi2->nom);
        }
        fprintf(fp, "%s 0 - 0 %s \n", equi1->nom, equi2->nom);
        /* Ecriture du score courant dans le fichier de déroulement et affichage du score. */
        printf(" Le score courant est de : %s %d - %d %s \n", equi1->nom, s_equip1, s_equip2, equi2->nom);
        int tour =ajoutdepoint();  /* Boucle de jeu pour marquer des points. */
        for (int i = 0; i < 6; i ++) {
            if(tour==0){
            s_equip1 = ancien1 + 1;
            usleep(s);
            tour=ajoutdepoint();}
            else{
            usleep(s);
            s_equip2 = ancien2 + 1;
            tour=ajoutdepoint();}
            fprintf(fp, "%s %d - %d %s \n", equi1->nom, s_equip1, s_equip2, equi2->nom);
            /* Ecriture du score courant dans le fichier de déroulement et affichage du score. */
            printf(" Le score courant est de : %s %d - %d %s \n", equi1->nom, s_equip1, s_equip2, equi2->nom);
            ancien1 = s_equip1;
            ancien2 = s_equip2;
        }
        Equipe gagnant;
         /* détermination du gagnant */
        if (s_equip1 > s_equip2){
            gagnant = *equi1;
            fprintf(fp, "  ##  %s a gagné ! ## \n", equi1->nom);
        }
        else if (s_equip2 > s_equip1){
            gagnant = *equi2;
            fprintf(fp, "  ##   %s a gagné ! ## \n", equi2->nom);
        }
        else {
            /*prolongation*/
            int resultat = rand() % 2;
            if (resultat == 0){
                gagnant = *equi1;
                gagnant.resultat = s_equip1;
                printf("  **  Prolongation %s a gagné ! avec un score de %d - %d **\n", equi1->nom , s_equip1 +1,s_equip2  );
                fprintf(fp, "  **  Prolongation %s a gagné ! avec un score de %d - %d **\n", equi1->nom , s_equip1 +1 ,s_equip2 );
            }
            else {
                gagnant = *equi2;
                gagnant.resultat = s_equip2;
                printf("  **  Prolongation %s a gagné ! avec un score de %d - %d **\n", equi2->nom , s_equip2 ,s_equip1+1 );
                fprintf(fp, "  **  Prolongation %s a gagné ! avec un score de %d - %d **\n", equi2->nom , s_equip2 ,s_equip1+1 );
            }
        }
        fclose(fp);

        close(pipefd[0]);   // * ferme l'extrémité de lecture du pipe                     
        write(pipefd[1], &gagnant, sizeof(Equipe)); // * envoie l'équipe gagnante au processus père
        close(pipefd[1]);  // * ferme l'extrémité d'écriture du pipe                      
        exit(0);
    }
    else if (pid > 0){
        close(pipefd[1]); // * ferme l'extrémité de lecture du pipe
        Equipe gagnant;
        read(pipefd[0], &gagnant, sizeof(Equipe)); 
        close(pipefd[0]);                      
        return gagnant;
    }
    else
    {
        perror("fork");
        exit(1);
    }
}

/**
*@brief Fonction qui mélange un tableau d'équipes de façon aléatoire
*@param equipe Un pointeur vers le tableau d'équipes à mélanger
*@param num_equi Le nombre d'équipes dans le tableau
*@return Un pointeur vers le tableau d'équipes mélangé
*La fonction utilise l'algorithme de mélange de Fisher-Yates pour mélanger les équipes.
*Elle parcourt le tableau et échange chaque élément avec un élément aléatoire du reste du tableau.
*Cela permet de créer un ordre aléatoire des équipes pour le tournoi. 
*/
Equipe *melange(Equipe *equipe, int num_equi){
    for (int i = 0; i < num_equi - 1; i++){
        int j = i + rand() / (RAND_MAX / (num_equi - i) + 1);
        Equipe temp = equipe[j];
        equipe[j] = equipe[i];
        equipe[i] = temp;
    }
    return equipe;
}

/**
*@brief Fonction qui permet à un thread de jouer un tournoi jusqu'à ce qu'il ne reste qu'une équipe gagnante
*@param params pointeur vers la structure de paramètres contenant les informations nécessaires pour jouer le tournoi
*@return un pointeur vers la liste finale d'équipes gagnantes
*/
void *jouer(void *params){
    int threadId = __sync_add_and_fetch(&threadCount, 1); // * génère un identifiant unique de thread
    srand(time(NULL) + threadId);
    Mpar *params_ptr = (Mpar *)params;
    int cote_equipe = params_ptr->cote_equipe;
    char* poul = params_ptr->half; 
    char* moitie;
    int spd = params_ptr->speedy;
    if (strcmp(poul,"right")){
        moitie = "1ere moitié";} else {moitie = "2eme moitié";} 
    Equipe *cote = params_ptr->cote; // * équipes
    int a = params_ptr->a; // * temps d'attente
    pthread_barrier_t *barrier = params_ptr->barrier; // * barrière de synchronisation
    pthread_barrier_t *end_barrier = params_ptr->end_barrier; // * barrière de synchronisation pour la fin
    melange(cote, cote_equipe); // * mélange les équipes
    while (cote_equipe > 1){
        int rounds = cote_equipe / 2; // * nombre de tours
        Equipe new_teams[(cote_equipe + 1) / 2]; // * nouvelle liste d'équipes
        int new_teams_index = 0;
        for (int j = 0; j < rounds; j++){ // * pour chaque tour
            Equipe equi1 = cote[j]; // * première équipe
            Equipe equi2 = cote[cote_equipe - 1 - j]; // * deuxième équipe
            Equipe gagnant = matchexec(&equi1, &equi2, poul,spd); // * exécute le match et obtient le gagnant
            new_teams[new_teams_index++] = gagnant; // * ajoute le gagnant à la nouvelle liste
            printf("\n ------------------------- \n Le gagnant du matche (%s) %s vs %s est %s  \n ------------------------- \n",moitie, equi1.nom, equi2.nom, gagnant.nom); // * affiche le gagnant
            sleep(a); // * attend un peu avant de continuer
        }
        cote_equipe = new_teams_index;
        for (int i = 0; i < cote_equipe; i++){
            cote[i] = new_teams[i];
        }
        // * Attend que l'autre thread atteigne ce point avant de continuer
        pthread_barrier_wait(barrier);
    }
    // * Attend que l'autre thread atteigne ce point avant de continuer
    pthread_barrier_wait(end_barrier);
    return cote; // * renvoie la liste finale des équipes
}


/**
*Détermine le gagnant final à partir de deux groupes d'équipes divisées en fonction des cotes.
*@param cote_equipe Le nombre total d'équipes
*@param cote La liste complète d'équipes
*@param nbrCote Le nombre de caractères pour le nom de chaque équipe
*@param a Le temps d'attente entre chaque match
*@param half 
*@param s La vitesse d'exécution des matchs
*@return L'équipe gagnante finale
*/
Equipe cote_gagnant(int cote_equipe, Equipe *cote, int nbrCote, int a, char* half, int s){

    // * *Divise les équipes en deux groupes en fonction des cotes
    int m_cote_equipe = cote_equipe / 2;
    Equipe *cote1 = cote;
    Equipe *cote2 = &cote[m_cote_equipe];

    pthread_barrier_t barrier, end_barrier;    // * *Crée les barrières de synchronisation
    pthread_barrier_init(&barrier, NULL, 2);
    pthread_barrier_init(&end_barrier, NULL, 2);

    // * Définit les paramètres pour chaque match
    Mpar match1_params = {"right",m_cote_equipe, cote1, nbrCote, a, &barrier, &end_barrier,s};
    Mpar match2_params = {"left",cote_equipe - m_cote_equipe, cote2, nbrCote, a, &barrier, &end_barrier,s};

    pthread_t match1_thread, match2_thread; // * Lance les matchs en parallèle
    pthread_create(&match1_thread, NULL, jouer, &match1_params);
    pthread_create(&match2_thread, NULL, jouer, &match2_params);


    Equipe *resultat1, *resultat2; // * Attend la fin des deux matchs
    pthread_join(match1_thread, (void **)&resultat1);
    pthread_join(match2_thread, (void **)&resultat2);

    Equipe gagnant1 = *resultat1;    // * Détermine le gagnant final à partir des gagnants de chaque match
    Equipe gagnant2 = *resultat2;
    Equipe gagnantfinal = matchexec(&gagnant1, &gagnant2, half,s);
    return gagnantfinal;
}

/**
*@brief Jouer une demi-finale en utilisant un processus enfant pour lancer la fonction cote_gagnant et récupérer le nom du champion
*@param arg Pointeur vers une structure ParThread contenant les arguments pour jouer la demi-finale
*@return void* Pointeur vers une chaîne de caractères contenant le nom du champion
*/
void *jouer_semifinal(void *arg){
    ParThread *thread_arg = (ParThread *)arg;    // * Convertir l'argument en pointeur de structure ParThread
    int taille_cote1 = thread_arg->taille;    // * Obtenir la taille du tableau d'équipe depuis la structure ParThread
    Equipe *cote1 = thread_arg->equipe;    // * Obtenir le tableau d'équipe depuis la structure ParThread
    int a = thread_arg->temps; // * Obtenir la valeur de temps depuis la structure ParThread
    char* poul = thread_arg->half; // * Obtenir le nom de la demi-finale depuis la structure ParThread
    int spd = thread_arg->speed;
    Equipe gagnant1; // * Créer une variable pour stocker l'équipe gagnante
    char nom_champion1[50]; // * Créer un tampon pour stocker le nom du champion
    // * Créer un pipe pour communiquer entre les processus parent et enfant
    int pipes[2];
    int status;
    if (pipe(pipes) == -1){ // * S'il y a eu une erreur lors de la création du pipe, afficher un message d'erreur et sortir
        perror("pipe");
        exit(1);
    }
    pid_t pid1; // * Créer un processus enfant
    switch (pid1 = fork()){
    case 0: // * C'est le processus enfant
        // * Obtenir l'équipe gagnante depuis la fonction cote_gagnant
        gagnant1 = cote_gagnant(taille_cote1, cote1, 1, a, poul,spd);
        // * Copier le nom de l'équipe gagnante dans la variable nom_champion1
        strcpy(nom_champion1, gagnant1.nom);
        // * Fermer le côté de lecture du pipe
        close(pipes[0]);
        // * Écrire le nom du champion dans le pipe
        write(pipes[1], &nom_champion1, sizeof(nom_champion1));
        // * Fermer le côté d'écriture du pipe et quitter le processus enfant
        close(pipes[1]);
        exit(0);
    case -1: // * S'il y a eu une erreur lors de la création du processus enfant, afficher un message d'erreur et sortir
        perror("Erreur fork");
        exit(EXIT_FAILURE);
    default: // * C'est le processus parent
        // * Attendre que le processus enfant se termine
        waitpid(pid1, &status, 0);
        // * Fermer le côté d'écriture du pipe
        close(pipes[1]); 
        // * Lire le nom du champion depuis le pipe
        read(pipes[0], &nom_champion1, sizeof(nom_champion1)); 
        // * Fermer le côté de lecture du pipe
        close(pipes[0]);                                     
    }
    // * Allouer de la mémoire pour le résultat et copier le nom du champion dans le résultat
    char *result = malloc(sizeof(nom_champion1)); 
    strcpy(result, nom_champion1);                
    return result;
}


/**
*@brief Compte le nombre de pays dans un fichier txt
*@param fp Pointeur vers le fichier texte
*@return Le nombre de pays dans le fichier txt
*/
int counter(FILE *fp){
    char line[TAILLEMAXLIGNE];    // * Déclarer une chaîne de caractères pour stocker une ligne
    int count = 0;    // * Initialiser un compteur à zéro
    
    // * Lire chaque ligne du fichier texte
    while (fgets(line, TAILLEMAXLIGNE, fp) != NULL) {
        // * Ignorer les lignes vides ou ne contenant que des sauts de ligne
        if (strlen(line) <= 1) {
            continue;
        }
        char *token = strtok(line, ",");    // * Séparer la ligne en tokens à chaque virgule et compter le nombre de tokens
        while (token != NULL){
            count++;
            token = strtok(NULL, ",");
        }
    }
    return count ;
}
/**
*@brief Retire les espaces au début et à la fin d'une chaîne de caractères.
*@param str La chaîne de caractères à traiter.
*/
void couper(char *str){
    int longeur = strlen(str);
    int i = 0, j = longeur - 1;
    // * Retirer les espaces avant la chaîne
    while (str[i] == ' ' && i < longeur){
        i++;
    }
    // * Retirer les espaces après la chaîne
    while (str[j] == ' ' && j >= 0){
        j--;
    }
    // * Déplacer la chaîne pour retirer les espaces
    for (int k = i; k <= j; k++){
        str[k - i] = str[k];
    }
    // * Ajouter le caractère nul pour terminer la chaîne
    str[j - i + 1] = '\0';
}
/**
*@brief Ouvre un fichier text contenant une liste d'équipes et renvoie un pointeur vers une structure nombreEquipe contenant toutes les équipes.
*@param filename Le nom du fichier text.
*@return Un pointeur vers une structure nombreEquipe contenant toutes les équipes.
*/
nombreEquipe *file_equipes(char *filename){
    char nom_equipe[TAILLEMAXLIGNE];
    char *token;
    FILE *fp = fopen(filename, "r");
    if (fp == NULL){
        if(strcmp(filename,"")){
        printf("Impossible d'ouvrir le fichier '%s'.\n", filename); }
        fp = fopen("equipes_default.txt", "r");
        if (fp == NULL){
            printf("Impossible d'ouvrir le fichier par défaut 'equipes_default.txt'.\n");
            exit(EXIT_FAILURE);
        }
        strcpy(filename, "equipes_default.txt"); // * Utiliser le fichier par défaut
    }

    int nbr_equipes = counter(fp);
    fclose(fp);

    if (nbr_equipes == 0){
        printf("Le fichier '%s' ne contient pas d'équipe.\n", filename);
        exit(EXIT_FAILURE);
    }

    // * Vérifier si le nombre d'équipes est une puissance de 2
    if ((nbr_equipes & (nbr_equipes - 1)) != 0){
        printf("Le nombre d'équipes n'est pas une puissance de 2. Le fichier par défaut 'equipes_default.txt' sera utilisé.\n");
        fp = fopen("equipes_default.txt", "r");
        if (fp == NULL) {
            printf("Impossible d'ouvrir le fichier par défaut 'equipes_default.txt'.\n");
            exit(EXIT_FAILURE);
        }
        strcpy(filename, "equipes_default.txt"); // * Utiliser le fichier par défaut
        nbr_equipes = counter(fp);
        fclose(fp);
    }
    // * gestion des erreurs
    nombreEquipe *nbretteam_ptr = malloc(sizeof(nombreEquipe));
    if (nbretteam_ptr == NULL) {
        printf("Erreur d'allocation de mémoire.\n");
        exit(EXIT_FAILURE);
    }
    memset(nbretteam_ptr, '\0', sizeof(nombreEquipe));
    nbretteam_ptr->equipe = malloc(nbr_equipes * sizeof(Equipe));
        // * gestion des erreurs

    if (nbretteam_ptr->equipe == NULL) {
        printf("Erreur d'allocation de mémoire.\n");
        exit(EXIT_FAILURE);
    }
    memset(nbretteam_ptr->equipe, '\0', nbr_equipes * sizeof(Equipe));
    // * gestion des erreurs

    fp = fopen(filename, "r");
    if (fp == NULL){
        printf("Impossible d'ouvrir le fichier '%s'.\n", filename);
        exit(EXIT_FAILURE);
    }

    fgets(nom_equipe, TAILLEMAXLIGNE, fp);

    token = strtok(nom_equipe, ",");
    int i = 0;
    while (token != NULL && i < nbr_equipes) {
        // * Retir
        couper(token);
        nbretteam_ptr->equipe[i] = initEquipe(token);
        token = strtok(NULL, ",");
        i++;
    }

    nbretteam_ptr->taille = nbr_equipes;
    fclose(fp);
    return nbretteam_ptr;
}
/**
*@brief Cette fonction calcule le nombre d'équipes dans un tableau d'équipes donné
*@param teams Le tableau d'équipes
*@return Le nombre d'équipes dans le tableau
*/
int getNumEquipes(Equipe *teams){
    int nbr_equipes = 0;
    while (teams[nbr_equipes].nom[0] != '\0'){
        nbr_equipes++;
    }
    return nbr_equipes;
}
/**
*@brief Lit le fichier d'options et renvoie les vitesses de simulation pour les matchs.
*@return int* Un pointeur vers un tableau d'entiers de taille 2 contenant les vitesses de simulation.
*La première valeur correspond au delai avant le lancement d'un match et la 2eme valeur correspond au temps entre chaque round d'un match(milisecondes)
*@note Le fichier d'options doit être nommé "options.txt" et se trouver dans le répertoire courant.
*/
int* option_match(){
    FILE *fptr;
    int* vitesse = malloc(2 * sizeof(int));

    fptr = fopen("options.txt", "r");

    // * Vérifie si le fichier a été ouvert avec succès
    if (fptr == NULL){
        printf("Erreur : impossible d'ouvrir le fichier\n");
    }

    fscanf(fptr, "%*s");
    fscanf(fptr, "%d", &vitesse[1]);
    fscanf(fptr, "%*s");
    fscanf(fptr, "%d", &vitesse[2]);
    fclose(fptr);
    return vitesse;
}


int main(int argc, char* argv[]){
    // * Initialisation des équipes
    nombreEquipe *nbretteam_ptr;
    if (argc > 1) {
        nbretteam_ptr = file_equipes(argv[1]);
    } else {
        printf("utilisation du fichier par défaut : equipes_default.txt \n");
        nbretteam_ptr = file_equipes("equipes_default.txt");
    }
        int nbr_equipes = nbretteam_ptr->taille;
    Equipe *teams = nbretteam_ptr->equipe;
    melange(teams, nbr_equipes);

    FILE *fp = fopen("deroulement.txt", "a"); // * Ouverture du fichier en mode lecture/écriture
    if (fp == NULL){ // * Le fp n'existe pas, on le crée et on l'ouvre en mode écriture
        fp = fopen("deroulement.txt", "a");
        if (fp == NULL) {
            printf("Erreur lors de la création du fichier.");
            exit(1);
        }
    }
    else { // * Le fp existe déjà, on supprime son contenu et on l'ouvre en mode écriture
        if (remove("deroulement.txt") != 0){
            printf("Erreur lors de la suppression du contenu du fichier.");
            exit(1);
        }
        fp = fopen("deroulement.txt", "a");
        if (fp == NULL) {
            printf("Erreur lors de l'ouverture du fichier en mode écriture.");
            exit(1);
        }
    }

    int taille_cote1 = nbr_equipes / 2;
    Equipe cote1[taille_cote1];
    printf("***** géneration des equipes  ****** \n");
    for (int i = 0; i < 3; i++) {
        printf("            ****\n");
        sleep(1);
    }
    
    printf("   1ere moitié :\n");
    for (int i = 0; i < taille_cote1; i++){
        cote1[i] = teams[i];
        printf("  %s / ", cote1[i].nom);
    }
         printf(" \n ###################################################################### \n");
   
    int taille_cote2 = nbr_equipes / 2;
    Equipe cote2[taille_cote2];
    printf("   2eme moitié :\n");
    for (int i = 0; i < taille_cote2; i++) {
        cote2[i] = teams[taille_cote2 + i];
        printf("  %s /  ", cote2[i].nom);
    }
         printf(" \n ###################################################################### \n \n");

    int* ops = option_match();
    int temps1 = ops[1];
    int spd = ops[2];
    ParThread *arg = malloc(sizeof(ParThread));
    arg->taille = taille_cote1;
    arg->half = "right";
    arg->equipe = cote1;
    arg->temps = temps1;
    arg->speed = spd;
    ParThread *arg2 = malloc(sizeof(ParThread));
    arg2->taille = taille_cote2;
    arg2->half = "left";
    arg2->equipe = cote2;
    arg2->temps = temps1;
    arg2->speed = spd;

    pthread_t tid1, tid2;
    char *nom_champion1, *nom_champion2;

    pthread_create(&tid1, NULL, &jouer_semifinal, arg);
    pthread_create(&tid2, NULL, &jouer_semifinal, arg2);
    // * Attendre la fin du thread et récupérer le résultat
    pthread_join(tid1, (void **)&nom_champion1);
    pthread_join(tid2, (void **)&nom_champion2);
    // * Afficher le gagnant de la première demi-finale
    printf(" ################### \n  Le gagnant de la semi final de la premiere moité : %s\n", nom_champion1);
    // * Afficher le gagnant de la deuxième demi-finale
    printf(" ################### \n  Le gagnant de la semi final de la seconde moité : %s\n ###################\n", nom_champion2);
    printf("--------------------------------------------\n--------------------------------------------\n--------------------------------------------\n");
    // * Finale
    // * afficher les finalistes avec les pipe
    printf(" Les finalistes sont :  %s et %s\n", nom_champion1, nom_champion2);
    Equipe finalist1;
    strcpy(finalist1.nom, nom_champion1);
    finalist1.resultat = 0;
    Equipe finalist2;
    strcpy(finalist2.nom, nom_champion2);
    finalist2.resultat = 0;
    printf("--------------------------------------------\n--------------------------------------------\n--------------------------------------------\n");

    // *    Finale simulation
    pid_t pid_final;
    char winner3_name[20];
    int pipes3[2];
    Equipe winner_final;
    int status;
    if (pipe(pipes3) == -1) {
        perror("pipe");
        exit(1);
    }
    switch (pid_final = fork()) {
    case 0:
        winner_final = matchexec(&finalist1, &finalist2,"F",spd);
        strcpy(winner3_name, winner_final.nom);
        close(pipes3[0]);                                      
        write(pipes3[1], &winner3_name, sizeof(winner3_name)); // * ecrire gagnant dans pipe
        close(pipes3[1]);                                      
        exit(0);
    case -1:
        perror("Erreur fork");
        exit(EXIT_FAILURE);

    default:
        waitpid(pid_final, &status, 0);

        close(pipes3[1]); 
        read(pipes3[0], &winner3_name, sizeof(winner3_name)); // * lire gagnant depuis pipe
        close(pipes3[0]);                                     
        printf(" \n ******************************* Le gagnant de la finale est : %s *******************************\n\n\n", winner3_name);
        fprintf(fp, " \n ******************************* Le gagnant de la finale est : %s *******************************\n\n\n", winner3_name);
        fclose(fp);

    }
    // * Terminer le programme
    free(arg);
    free(nom_champion1);
    free(arg2);
    free(nom_champion2);

    return 0;
}