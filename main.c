#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>

#define TAILLE 9
#define VIDE 0
#define grille_w 600
#define grille_h 600
#define grille_y 40
#define xl 2
#define xL 3.5
#define textH 45
#define textW 45
#define case_choix_h 80
#define case_choix_w 55
#define liste_choix_y 120
#define button_w 150
#define button_h 50
#define button_y 300

// Structure pour le jeu
typedef struct {
    char nom_joueur[50];
    int score_actuel;
    int dernier_score;
    int meilleur_score;
    int niveau;
    int partie;
    int tentatives_help;
    Uint32 start_time;
} Jeu;

// Définition des états du jeu
typedef enum {
    PAGE_MENU,
    PAGE_JEU,
    PAGE_FIN_PARTIE
} Page;

void demander_nom_joueur(Jeu *jeu) {
    printf("Entrez votre nom : ");
    scanf("%s", jeu->nom_joueur);
}

// Fonctions pour le jeu
void sauvegarder_score(Jeu *jeu) {
    FILE *fichier = fopen("score.txt", "a");
    if (fichier) {
        if (jeu->score_actuel > jeu->dernier_score) {
            jeu->dernier_score = jeu->score_actuel;
        }
        fprintf(fichier, "Score Actuel: %d\nDernier Score: %d\nMeilleur Score: %d\n",
                jeu->score_actuel, jeu->dernier_score, jeu->meilleur_score);
        fclose(fichier);
    }
}

void charger_score(Jeu *jeu) {
    FILE *fichier = fopen("score.txt", "r");
    if (fichier) {
        fscanf(fichier, "Score Actuel: %d\nDernier Score: %d\nMeilleur Score: %d\n",
               &jeu->score_actuel, &jeu->dernier_score, &jeu->niveau);
        fclose(fichier);
    } else {
        jeu->score_actuel = 0;
        jeu->dernier_score = 0;
        jeu->niveau = 1;
    }
}

void update_score(Jeu *jeu, bool correcte) {
    if (correcte) {
        jeu->score_actuel += 10;
    }
}

void restart_game(Jeu *jeu) {
    jeu->score_actuel = 0;
    jeu->tentatives_help = 3;
    jeu->niveau = 1;
    jeu->partie = 1;
    jeu->start_time = SDL_GetTicks();
    sauvegarder_score(jeu);
}

void utiliser_help(Jeu *jeu) {
    if (jeu->tentatives_help > 0) {
        jeu->tentatives_help--;  // Diminue le nombre de tentatives gratuites
    } else {
        jeu->score_actuel -= 20; // Retire des points si toutes les aides ont été utilisées
        if (jeu->score_actuel < 0) jeu->score_actuel = 0;
    }
}

void appliquer_bonus_temps(Jeu *jeu) {
    Uint32 temps_ecoule = (SDL_GetTicks() - jeu->start_time) / 1000; // Temps en secondes
    if (temps_ecoule < 60) {
        jeu->score_actuel += 100; // Bonus de 100 points si la partie est terminée en moins de 60 secondes
    } else if (temps_ecoule < 120) {
        jeu->score_actuel += 50; // Bonus de 50 points si terminé en moins de 2 minutes
    }
}

// Fonction pour passer à l'étape suivante
void prochaine_etape(Jeu *jeu, SDL_Renderer *renderer, TTF_Font *font) {
    appliquer_bonus_temps(jeu);  // Appliquer le bonus avant de sauvegarder
    jeu->dernier_score = jeu->score_actuel;
    sauvegarder_score(jeu);

    if (jeu->partie < 3) {
        jeu->partie++;
    } else {
        jeu->partie = 1;
        jeu->niveau++;
    }
}

// Fonction pour afficher l'animation de victoire
void afficher_animation(SDL_Renderer *renderer, TTF_Font *font) {
    SDL_Color color = {0, 255, 0}; // Vert pour la victoire
    afficher_message(renderer, font, "🎉🥳 Victoire ! 🎉🥳", 250, 250);

    // Clignotement du message
    for (int i = 0; i < 3; i++) {
        SDL_Delay(500); // Pause de 0.5 seconde
        SDL_SetRenderDrawColor(renderer, 245, 245, 245, 255); // Fond blanc
        SDL_RenderClear(renderer);
        SDL_RenderPresent(renderer);
        SDL_Delay(500);
        afficher_message(renderer, font, "🎉🥳 Victoire ! 🎉🥳", 250, 250);
        SDL_RenderPresent(renderer);
    }

    SDL_Delay(2000);  // Pause de 2 secondes avant de continuer
}

// Fonction pour afficher le chronomètre
void afficher_temps(SDL_Renderer *renderer, TTF_Font *font, Uint32 start_time) {
    Uint32 temps_ecoule = (SDL_GetTicks() - start_time) / 1000; // Temps en secondes
    char temps_str[50];
    snprintf(temps_str, sizeof(temps_str), "Temps écoulé : %d sec", temps_ecoule);
    afficher_message(renderer, font, temps_str, 20, 20);
}
// Fonction pour afficher le score
void afficher_score(SDL_Renderer *renderer, TTF_Font *font, Jeu *jeu) {
    char score_str[100];
    snprintf(score_str, sizeof(score_str), "Score: %d | Dernier: %d",
             jeu->score_actuel, jeu->dernier_score);
    afficher_message(renderer, font, score_str, 20, 50);
}

void afficher_score_final(SDL_Renderer *renderer, TTF_Font *font, Jeu *jeu) {
    char message[100];

    if (jeu->score_actuel < 200) {
        sprintf(message, "%s - Score: %d - Niveau: Débutant", jeu->nom_joueur, jeu->score_actuel);
    } else if (jeu->score_actuel < 500) {
        sprintf(message, "%s - Score: %d - Niveau: Intermédiaire", jeu->nom_joueur, jeu->score_actuel);
    } else {
        sprintf(message, "%s - Score: %d - Niveau: Expert", jeu->nom_joueur, jeu->score_actuel);
    }

    afficher_message(renderer, font, message, 200, 200);
    SDL_Delay(3000);
}

void enregistrer_meilleur_score(Jeu *jeu) {
    FILE *fichier = fopen("meilleurs_scores.txt", "a");
    if (fichier) {
        fprintf(fichier, "%s %d\n", jeu->nom_joueur, jeu->score_actuel);
        fclose(fichier);
    }
}

// Vérification de fin du jeu
void verifier_fin_du_jeu(Jeu *jeu, SDL_Renderer *renderer, TTF_Font *font) {
    if (jeu->niveau > 3) {
        enregistrer_meilleur_score(jeu);
        afficher_score_final(renderer, font, jeu);
        afficher_meilleurs_scores(renderer, font);
    }
}

void afficher_meilleurs_scores(SDL_Renderer *renderer, TTF_Font *font) {
    FILE *fichier = fopen("meilleurs_scores.txt", "r");
    if (!fichier) return;

    char ligne[100];
    int y = 100;

    while (fgets(ligne, sizeof(ligne), fichier)) {
        afficher_message(renderer, font, ligne, 200, y);
        y += 30;
    }

    fclose(fichier);
    SDL_Delay(5000);
}

void afficher_niveau(SDL_Window *window, SDL_Renderer *renderer, TTF_Font *font, Jeu jeu) {
    int width, height;
    SDL_GetWindowSize(window, &width, &height);
    char niveau[50];
    snprintf(niveau, sizeof(niveau), "Niveau : %d", jeu.niveau);
    afficher_message(renderer, font, niveau, width - 210, 20);
}

void afficher_partie(SDL_Window *window, SDL_Renderer *renderer, TTF_Font *font, Jeu jeu) {
    int width, height;
    SDL_GetWindowSize(window, &width, &height);
    char partie[50];
    snprintf(partie, sizeof(partie), "Partie : %d", jeu.partie);
    afficher_message(renderer, font, partie, width - 210, 50);
}

// Fonction pour afficher un message dans la fenêtre SDL
void afficher_message(SDL_Renderer *renderer, TTF_Font *font, const char *message, int x, int y) {
    SDL_Color textColor = {0, 0, 0};  // Couleur noire

    SDL_Surface *textSurface = TTF_RenderUTF8_Solid(font, message, textColor);
    SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_Rect textRect = {x, y, textSurface->w, textSurface->h};
    SDL_Rect rectVide = {x, y, textSurface->w, textSurface->h}; // rectangle vide pour écraser l'affichage avant

    SDL_SetRenderDrawColor(renderer, 245, 245, 245, 255); // ecraser l'affichage
    SDL_RenderFillRect(renderer, &rectVide);

    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}

void afficher_nombre(SDL_Renderer *renderer, TTF_Font *font, SDL_Rect caseJeu, int nombre, bool isInitial, bool conflit) {
    SDL_Color couleur;

    if (isInitial) {
        couleur = (SDL_Color){130, 177, 255, 255};  // 🔵 BLEU pour les nombres initiaux
    } else if (conflit) {
        couleur = (SDL_Color){237, 87, 87, 255};   // 🔴 ROUGE si conflit
    } else {
        couleur = (SDL_Color){0, 180, 0, 255};     // 🟢 VERT pour un nombre valide
    }

    char texte[2];
    sprintf(texte, "%d", nombre);

    SDL_Surface *surface = TTF_RenderText_Solid(font, texte, couleur);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

    SDL_Rect rect = {
        caseJeu.x + (caseJeu.w  - textW) / 2,
        caseJeu.y + (caseJeu.h - textH) / 2,
        textW,
        textH
    };

    SDL_RenderCopy(renderer, texture, NULL, &rect);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

bool verifier_conflit(int grille[TAILLE][TAILLE], int ligne, int colonne, int nombre) {
    // Vérifier la ligne et la colonne
    for (int i = 0; i < TAILLE; i++) {
        if (grille[ligne][i] == nombre || grille[i][colonne] == nombre) {
            return true;
        }
    }
    // Vérifier la sous-grille 3x3
    int debutLigne = (ligne / 3) * 3;
    int debutColonne = (colonne / 3) * 3;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (grille[debutLigne + i][debutColonne + j] == nombre) {
                return true;
            }
        }
    }
    return false;
}

void vider_cases(int grille[TAILLE][TAILLE], int niveau, int partie) {
    int cases_vides;
    switch (niveau) {
        case 1:
            cases_vides = (partie == 1) ? 15 : (partie == 2) ? 20 : 25;
            break;
        case 2:
            cases_vides = (partie == 1) ? 30 : (partie == 2) ? 35 : 40;
            break;
        case 3:
            cases_vides = (partie == 1) ? 45 : (partie == 2) ? 50 : 55;
            break;
    }

    int count = 0;
    while(count <cases_vides){
        int ligne = rand()%TAILLE;
        int colonne = rand()%TAILLE;
        if(grille[ligne][colonne]!=VIDE){
            grille[ligne][colonne]=VIDE;
            count ++;
        }
    }
}

void afficher_grille(int grille[TAILLE][TAILLE]);
bool valide(int grille[TAILLE][TAILLE], int ligne, int colonne, int numero);
bool remplir_grille(int grille[TAILLE][TAILLE], int ligne, int colonne);
void vider_cases(int grille[TAILLE][TAILLE], int niveau, int partie);
void tracerGrille(SDL_Rect **tab_cases, int grille[TAILLE][TAILLE], SDL_Renderer *renderer, SDL_Window *window, TTF_Font *font);
void tracerNumeros(SDL_Rect **tab_cases, int grille[TAILLE][TAILLE], SDL_Renderer *renderer, TTF_Font *font);
void tracer_bloc(int ligne_i, int colonne_j, int caseWidth, int caseHeight, int blocWidth, int blocHeight, int grille_x, SDL_Rect **tab_cases, SDL_Renderer *renderer);
void tracerListeDesChoix(SDL_Rect *cases_choix, SDL_Renderer *renderer, SDL_Window *window, TTF_Font *font);
void changercouleur(SDL_Renderer *renderer, SDL_Rect caseClique);
void reinitialiserGrilleCouleurs(SDL_Renderer *renderer, SDL_Rect **tab_cases);
int* recupererCase(SDL_Rect **tab_cases, int *caseClique, int position_x, int position_y);
int recupererCaseChoix(SDL_Rect *cases_choix, int position_x, int position_y);

int main(int argc, char *argv[]) {
    srand(time(NULL));

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("Erreur d'initialisation de SDL: %s\n", SDL_GetError());
        return 1;
    }

    if (TTF_Init() == -1) {
        printf("Erreur TTF_Init: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow(
        "Sudoku SDL",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        800, 600,
        SDL_WINDOW_RESIZABLE
    );

    if (!window) {
        printf("Erreur lors de la création de la fenêtre: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_MaximizeWindow(window);
    int width, height;
    SDL_GetWindowSize(window, &width, &height);

    SDL_Surface *icon = SDL_LoadBMP("icon.bmp");
    if (icon) {
        SDL_SetWindowIcon(window, icon);
        SDL_FreeSurface(icon);
    } else {
        printf("Erreur lors du chargement de l'icône: %s\n", SDL_GetError());
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Erreur lors de la création du renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    TTF_Font *font = TTF_OpenFont("Arial.ttf", 32);
    Uint32 tempActuel = SDL_GetTicks();
    const Uint32 DUREE_ANIMATION_VICTOIRE = 10000;
    const Uint32 DUREE_ANIMATION_NIVEAU_SUIVANT = 15000;

    // Initialisation du jeu
    Page page = PAGE_MENU;

    //chargement du jeu
    Jeu jeu = {0,0,1,1,0, SDL_GetTicks()};
    jeu.dernier_score = 0;
    jeu.meilleur_score = 0;
    jeu.tentatives_help = 0;
    demander_nom_joueur(&jeu);
    charger_score(&jeu);
    restart_game(&jeu);

    int showLevels = 0; // Variable pour afficher ou masquer les boutons de niveaux
    int showParts = 0;  // Variable pour afficher ou masquer les boutons de parties
    int selectedLevel = 0; // Niveau sélectionné (1, 2 ou 3)

    // numeros de la grille
    int grille[TAILLE][TAILLE] = {0};

    // grille de jeu
    SDL_Rect **tab_cases = (SDL_Rect*)malloc(9 * sizeof(SDL_Rect*));
    for (int i = 0; i < 9; i++) {
        tab_cases[i] = (SDL_Rect *)malloc(9 * sizeof(SDL_Rect));
    }

    // cases du jeu
    SDL_Rect *cases_choix = (SDL_Rect*)malloc(9 * sizeof(SDL_Rect));

    // bouton effacer
    int button_effacer_x = (width - grille_w)/2;
    SDL_Rect buttonEffacer = {button_effacer_x, height - button_y, button_w, button_h};

    // bouton help
    int button_help_x = (width - grille_w)/2 + grille_w - button_w;
    SDL_Rect buttonHelp = {button_help_x, height - button_y, button_w, button_h};

    int *caseClique = NULL;
    int caseModifie[2] = {-1, -1};
    int running = 1;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            } else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_h: // Appuyer sur "H" pour utiliser "Help"
                        utiliser_help(&jeu);
                        break;
                    case SDLK_r: // Appuyer sur "R" pour redémarrer
                        restart_game(&jeu);
                        break;
                    case SDLK_SPACE: // Appuyer sur "Espace" pour terminer une partie
                        tempActuel = SDL_GetTicks();
                        page = PAGE_FIN_PARTIE;
                        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Fond noir
                        SDL_RenderClear(renderer);

                        break;
                }
            } else if (event.type == SDL_MOUSEBUTTONDOWN) {
                int position_x = event.button.x;
                int position_y = event.button.y;

                if(page == PAGE_MENU) {
                    int position_x, position_y; // Déclaration de position_x et position_y
                    SDL_GetMouseState(&position_x, &position_y); // Récupération de la position de la souris

                    // Vérifier si l'utilisateur a cliqué sur le bouton "Niveaux"
                    if (!showLevels && isMouseOverButton(position_x, position_y, window, 250, 100, 50)) {
                        showLevels = 1; // Afficher les boutons de niveaux
                    }
                    // Vérifier si l'utilisateur a cliqué sur l'un des boutons de niveaux
                    else if (showLevels && !showParts) {
                        if (isMouseOverButton(position_x, position_y, window, 320, 100, 50)) {
                            selectedLevel = 1;
                            jeu.niveau = 1;
                            showParts = 1; // Afficher les boutons de parties pour le niveau 1
                        } else if (isMouseOverButton(position_x, position_y, window, 390, 100, 50)) {
                            selectedLevel = 2;
                            jeu.niveau = 2;
                            showParts = 1; // Afficher les boutons de parties pour le niveau 2
                        } else if (isMouseOverButton(position_x, position_y, window, 460, 100, 50)) {
                            selectedLevel = 3;
                            jeu.niveau = 3;
                            showParts = 1; // Afficher les boutons de parties pour le niveau 3
                        }
                    }
                    // Vérifier si l'utilisateur a cliqué sur l'un des boutons de parties
                    else if (showParts) {
                        if (isMouseOverButton(position_x, position_y, window, 320, 100, 50)) {
                            displayPartContent(window, renderer, font, "Partie 1");
                            jeu.partie = 1;
                        } else if (isMouseOverButton(position_x, position_y, window, 390, 100, 50)) {
                            displayPartContent(window, renderer, font, "Partie 2");
                            jeu.partie = 2;
                        } else if (isMouseOverButton(position_x, position_y, window, 460, 100, 50)) {
                            displayPartContent(window, renderer, font, "Partie 3");
                            jeu.partie = 3;
                        }
                        page = PAGE_JEU; // passer au jeu
                        intialiserPartie(window, renderer, font, tab_cases, grille, cases_choix, &jeu, buttonHelp, buttonEffacer);
                    }
                }
                else if(page == PAGE_JEU) {
                    caseClique = recupererCase(tab_cases, caseClique, position_x, position_y);
                    if (caseClique != NULL) {
                        tracerGrille(tab_cases, grille, renderer, window, font);
                        changercouleur(renderer, tab_cases[caseClique[0]][caseClique[1]]);
                        tracerNumeros(tab_cases, grille, renderer, font);
                    }

                    int caseChoix = recupererCaseChoix(cases_choix, position_x, position_y);
                    if (caseChoix != 0 && caseClique != NULL) {
                        int tab_case_i = caseClique[0];
                        int tab_case_j = caseClique[1];
                        if (grille[tab_case_i][tab_case_j] == 0) {
                            grille[tab_case_i][tab_case_j] = caseChoix;
                            bool conflit = verifier_conflit(grille, tab_case_i, tab_case_j, caseChoix);
                            grille[tab_case_i][tab_case_j] = caseChoix;
                            SDL_SetRenderDrawColor(renderer, 245, 245, 220, 255);
                            SDL_RenderFillRect(renderer, &tab_cases[tab_case_i][tab_case_j]);
                            afficher_nombre(renderer, font, tab_cases[tab_case_i][tab_case_j], caseChoix, false, conflit);
                            SDL_RenderPresent(renderer);
                            caseClique = NULL;
                            update_score(&jeu, conflit); // Mise à jour du score

                            // on stocke les coordonnées de la case modifiée
                            caseModifie[0] = tab_case_i;
                            caseModifie[1] = tab_case_j;

                            if(partieTermine(grille)) { // si la grille est complete, on passe à la partie suivante
                                tempActuel = SDL_GetTicks();
                                prochaine_etape(&jeu, renderer, font);
                                page = PAGE_FIN_PARTIE;
                                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Fond noir
                                SDL_RenderClear(renderer);
                            }
                        }
                    }
                    //gérér le bouton supprimer
                    handleSupprimerButtonClick(window, renderer, font ,buttonEffacer, position_x, position_y, caseModifie[0], caseModifie[1], tab_cases, grille, caseModifie);

                }
            }
        }

        if(page == PAGE_MENU) {
            intialiserMenu(window, renderer, font, showLevels, showParts);
        }
        if(page == PAGE_JEU) {
            // Afficher le score et le temps dans la fenêtre
            afficher_score(renderer, font, &jeu);
            afficher_temps(renderer, font, jeu.start_time);
        }
        if(page == PAGE_FIN_PARTIE) {
            Uint32 delai =  SDL_GetTicks() - tempActuel;

            if (jeu.niveau > 3) {
                afficher_message(renderer, font, "🎊🎉 Fin du jeu ! 🎉🎊", 250, 250);
                verifier_fin_du_jeu(&jeu, renderer, font);
            }
            else if(delai < DUREE_ANIMATION_VICTOIRE) {
               afficher_animation(renderer, font);
            }
            else if(delai == DUREE_ANIMATION_VICTOIRE) {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Fond noir
                SDL_RenderClear(renderer);
            }
            else if(delai < DUREE_ANIMATION_NIVEAU_SUIVANT && delai > DUREE_ANIMATION_VICTOIRE) {
                afficher_message(renderer, font, "Passage à l'étape suivante !", 250, 250);
            }
            else {
                page = PAGE_JEU;
                intialiserPartie(window, renderer, font, tab_cases, grille, cases_choix, &jeu, buttonHelp, buttonEffacer);
            }


        }
        // Mettre à jour l'écran
        SDL_RenderPresent(renderer);
        // Petite pause pour �viter de surcharger le CPU
        SDL_Delay(16);
        }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}


// Fonctions existantes pour la grille de Sudoku
void afficher_grille(int grille[TAILLE][TAILLE]){
    for(int i=0;i<TAILLE;i++){
        for(int j=0;j<TAILLE;j++){
            printf("%d",grille[i][j]);

        }

        printf("\n");

    }
    printf("\n");
}

bool valide(int grille[TAILLE][TAILLE],int ligne,int colonne,int numero){
   for(int i=0;i<TAILLE;i++){
       if(grille[ligne][i] == numero || grille[i][colonne] == numero){
           return false ;

       }
   }
   //verification de la sous grille 3x3
   int debut_ligne = ligne -ligne%3;
   int debut_colonne = colonne - colonne%3;
   for(int i=0;i<3;i++){
       for(int j =0;j<3;j++){
           if(grille[i+debut_ligne][j+debut_colonne]==numero){
               return false;

           }
       }
   }
   return true;
}

bool remplir_grille(int grille[TAILLE][TAILLE],int ligne,int colonne){
    if(ligne==TAILLE){
        return true;
    }
    if(colonne ==TAILLE){
        return remplir_grille(grille,ligne + 1,0);
    }
    if(grille[ligne][colonne]!=0){
        return remplir_grille(grille,ligne,colonne + 1);
    }
    for(int numero =1;numero<=TAILLE;numero++){
        if(valide(grille,ligne,colonne,numero)){
            grille[ligne][colonne]=numero;
            if(remplir_grille(grille,ligne,colonne + 1)){
                return true;
            }
            grille[ligne][colonne]=VIDE;
        }
    }
    return false ;
}

int partieTermine(int grille[TAILLE][TAILLE]) {
    for (int i = 0; i < TAILLE; i++) {
        for (int j = 0; j < TAILLE; j++) {
              if(grille[i][j] == 0) return 0;
        }
    }

    return 1;
}
void intialiserPartie(SDL_Window *window, SDL_Renderer * renderer, TTF_Font *font, SDL_Rect **tab_cases, int grille[TAILLE][TAILLE],
                      SDL_Rect *cases_choix, Jeu *jeu, SDL_Rect buttonHelp, SDL_Rect buttonEffacer) {
    int width, height;
    SDL_GetWindowSize(window, &width, &height);

    // coordon�s de la case grise
    SDL_SetRenderDrawColor(renderer, 245, 245, 245, 255);
    SDL_RenderClear(renderer);

    if (remplir_grille(grille, 0, 0)) {
        printf("Grille générée avec succès :\n");
        afficher_grille(grille);
    } else {
        printf("Erreur dans la génération de la grille.\n");
        return 1;
    }

    vider_cases(grille, jeu->niveau, jeu->partie);
    printf("\nGrille de Sudoku jouable:\n");
    afficher_grille(grille);
    tracerGrille(tab_cases, grille, renderer, window, font);
    tracerListeDesChoix(cases_choix, renderer, window, font);

    jeu->start_time = SDL_GetTicks();
    afficher_niveau(window, renderer, font, *jeu);
    afficher_partie(window, renderer, font, *jeu);


    afficherButton(renderer, font, "EFFACER", buttonEffacer);
    afficherButton(renderer, font, "HELP", buttonHelp);

}

void intialiserMenu(SDL_Window *window, SDL_Renderer * renderer, TTF_Font *font, int showLevels, int showParts) {
     // Effacer l'écran
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Fond noir
    SDL_RenderClear(renderer);

    // Dessiner le bouton "Niveaux"
    drawButton(window, renderer, font, "Niveaux", 350, 250, 100, 50);

    // Dessiner les boutons de niveaux si showLevels est vrai
    if (showLevels && !showParts) {
        drawButton(window, renderer, font, "Niveau 1", 350, 320, 100, 50);
        drawButton(window, renderer, font, "Niveau 2", 350, 390, 100, 50);
        drawButton(window, renderer, font, "Niveau 3", 350, 460, 100, 50);
    }

    // Dessiner les boutons de parties si showParts est vrai
    if (showParts) {
        drawButton(window, renderer, font, "Partie 1", 350, 320, 100, 50);
        drawButton(window, renderer, font, "Partie 2", 350, 390, 100, 50);
        drawButton(window, renderer, font, "Partie 3", 350, 460, 100, 50);
    }
}

void tracerGrille(SDL_Rect **tab_cases, int grille[TAILLE][TAILLE], SDL_Renderer * renderer, SDL_Window *window, TTF_Font *font) {
        // Obtenir les dimensions de la fen�tre
        int width, height;
        SDL_GetWindowSize(window, &width, &height);

        // coordon�s de la case grise
        int grille_x = (width - grille_w)/2;

        // Taille des petits carreaux (largeur et hauteur des cases)
        int caseWidth = ( grille_w - 6*xl - 2*xL ) / 9; // Largeur d'un carreau
        int caseHeight = ( grille_h - 6*xl - 2*xL ) / 9; // Hauteur d'un carreau

        //taille des blocs
        int blocWidth = (3 * caseWidth + 2 * xl);
        int blocHeight = (3 * caseHeight + 2 * xl);

        // rectangle noir (le cadre de la grille)
        SDL_Rect borderFill_1 = {grille_x - 5, grille_y - 5, grille_w + 10, grille_h + 10};
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Bordure noire
        SDL_RenderFillRect(renderer, &borderFill_1);

        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                if (i==0)
                {

                    if ( j == 2 )
                    {
                        tracer_bloc(i, j, caseWidth, caseHeight, blocWidth, blocHeight, grille_x, tab_cases, renderer);

                    }
                    else
                    {
                        tracer_bloc(i, j, caseWidth, caseHeight, blocWidth, blocHeight, grille_x, tab_cases, renderer);

                    }

                }
                else
                {
                    if (j==2)
                    {
                       tracer_bloc(i, j, caseWidth, caseHeight, blocWidth, blocHeight, grille_x, tab_cases, renderer);

                    }
                    else
                    {
                        tracer_bloc(i, j, caseWidth, caseHeight, blocWidth, blocHeight, grille_x, tab_cases, renderer);

                    }
                }
            }
        }
        tracerNumeros(tab_cases, grille, renderer, font);

}

void tracerNumeros(SDL_Rect **tab_cases, int grille[TAILLE][TAILLE], SDL_Renderer * renderer, TTF_Font *font) {
        for (int i=0; i<9; i++)
        {
            for (int j=0; j<9; j++)
            {
                int num = grille[i][j];
                char* numChar;

                asprintf(&numChar, "%d", num);
                SDL_Rect caseRect = tab_cases[i][j];
                if(num==0)
                {
                    continue;
                }
                else
                {
                SDL_Color textColor = {0, 0, 0, 255};

                SDL_Surface *textSurface = TTF_RenderText_Solid(font, numChar, textColor);

                SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

                SDL_Rect textRect = {
                    caseRect.x + (caseRect.w  - textW) / 2,
                    caseRect.y + (caseRect.h - textH) / 2,
                    textW,
                    textH
                };

                SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

                }
            }
    }
}

void tracer_bloc(int ligne_i, int colonne_j, int caseWidth, int caseHeight, int blocWidth, int blocHeight, int grille_x, SDL_Rect **tab_cases, SDL_Renderer * renderer)
{
        for (int i = 0; i < 3; i++)
        { // Ligne
           if (i!=2)
           {
               // ligne horizontale entre une ligne des petites cases
            SDL_Rect ligne_horizental = {
            grille_x + colonne_j * blocWidth + (colonne_j + 1) * xL,  // Position X
            grille_y + (i+1)* caseHeight + i*xl + ligne_i * blocHeight + ligne_i * xL +3, // Position Y
            3*caseWidth + 2 * xl ,                    // Largeur
            xl                   // Hauteur
            };
            SDL_SetRenderDrawColor(renderer, 192, 192, 192, 255); // Gris clair pour les carreaux
            SDL_RenderFillRect(renderer, &ligne_horizental); // Dessiner un carreau
            }

            for (int j = 0; j < 3; j++)
            {
                if (i==0)
                {

                    if ( j == 2 )
                    {
                        SDL_Rect petite_case = {
                        grille_x + j * caseWidth + j * xl + colonne_j * blocWidth + (colonne_j + 1) * xL,  // Position X
                        grille_y + i * caseHeight + ligne_i * blocHeight + (ligne_i+1) * xL, // Position Y
                        caseWidth ,                    // Largeur
                        caseHeight                     // Hauteur
                        };
                        SDL_SetRenderDrawColor(renderer, 245, 245, 220, 255); // Gris clair pour les carreaux
                        SDL_RenderFillRect(renderer, &petite_case); // Dessiner un carreau
                        tab_cases[3*ligne_i + i][3*colonne_j + j] = petite_case;


                    }
                    else
                    {

                        SDL_Rect petite_case = {
                        grille_x + j * caseWidth + j * xl + colonne_j * blocWidth + (colonne_j + 1) * xL ,  // Position X
                        grille_y + i * caseHeight + ligne_i * blocHeight + (ligne_i+1) * xL , // Position Y
                        caseWidth ,                    // Largeur
                        caseHeight                   // Hauteur
                        };
                        SDL_SetRenderDrawColor(renderer, 245, 245, 220, 255); // Gris clair pour les carreaux
                        SDL_RenderFillRect(renderer, &petite_case); // Dessiner un carreau
                        tab_cases[3*ligne_i + i][3*colonne_j + j] = petite_case;

                        // mettre  une ligne entre chaque deux cases
                        SDL_Rect ligne = {
                        grille_x + (j+1) * caseWidth + j * xl + colonne_j * blocWidth + (colonne_j + 1) * xL ,  // Position X
                        grille_y + i * caseHeight + ligne_i * blocHeight + (ligne_i+1) * xL , // Position Y
                        xl ,                    // Largeur
                        caseHeight                   // Hauteur
                        };
                        SDL_SetRenderDrawColor(renderer, 192, 192, 192, 255); // Gris clair pour les lignes entre les petites cases
                        SDL_RenderFillRect(renderer, &ligne);
                    }
                }
                else{
                    if ( j == 2 )
                    {
                        SDL_Rect petite_case = {
                        grille_x + j * caseWidth + j * xl + colonne_j * blocWidth + (colonne_j + 1) * xL,  // Position X
                        grille_y + i * caseHeight + i*xl + ligne_i * blocHeight + (ligne_i+1) * xL, // Position Y
                        caseWidth ,                    // Largeur
                        caseHeight                     // Hauteur
                        };
                        SDL_SetRenderDrawColor(renderer, 245, 245, 220, 255); // Gris clair pour les carreaux
                        SDL_RenderFillRect(renderer, &petite_case); // Dessiner un carreau
                        tab_cases[3*ligne_i + i][3*colonne_j + j] = petite_case;

                    }else{

                        SDL_Rect petite_case = {
                        grille_x + j * caseWidth + j * xl + colonne_j * blocWidth + (colonne_j + 1) * xL,  // Position X
                        grille_y + i * caseHeight + i*xl + ligne_i * blocHeight + (ligne_i+1) * xL , // Position Y
                        caseWidth ,                    // Largeur
                        caseHeight                   // Hauteur
                        };
                        SDL_SetRenderDrawColor(renderer, 245, 245, 220, 255); // Gris clair pour les carreaux
                        SDL_RenderFillRect(renderer, &petite_case); // Dessiner un carreau
                        tab_cases[3*ligne_i + i][3*colonne_j + j] = petite_case;

                        // mettre  une ligne entre chaque deux cases
                        SDL_Rect ligne = {
                        grille_x + (j+1) * caseWidth + j * xl + colonne_j * blocWidth + (colonne_j + 1) * xL ,  // Position X
                        grille_y + i * caseHeight + i*xl + ligne_i * blocHeight + (ligne_i + 1) * xL , // Position Y
                        xl ,                    // Largeur
                        caseHeight                   // Hauteur
                        };
                        SDL_SetRenderDrawColor(renderer, 192, 192, 192, 255); // Gris clair pour les lignes entre les petites cases
                        SDL_RenderFillRect(renderer, &ligne);

                    }


                    }
                }
            }
}

void tracerListeDesChoix(SDL_Rect *cases_choix, SDL_Renderer * renderer, SDL_Window *window, TTF_Font *font) {
        int width, height;
        SDL_GetWindowSize(window, &width, &height);

        // coordon�s de la case grise
        int liste_choix_x = (width - grille_w)/2;
        int espace_entre_cases = (grille_w - case_choix_w * 9)/8;

        for (int i = 0; i < 9; i++) {
                int position_x = liste_choix_x + i * case_choix_w + i * espace_entre_cases;
                char* numChar;
                asprintf(&numChar, "%d", i + 1);

                SDL_Rect case_choix = {position_x, height - liste_choix_y, case_choix_w, case_choix_h};
                cases_choix[i] = case_choix;
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                SDL_RenderFillRect(renderer, &case_choix);

                // tracer le numeros
                SDL_Color grandTextColor = {17, 67, 196, 255};
                SDL_Color petitTextColor = {0, 0, 0, 255};

                SDL_Surface *grandTextTextSurface = TTF_RenderText_Solid(font, numChar, grandTextColor);
                SDL_Texture *grandTextTexture = SDL_CreateTextureFromSurface(renderer, grandTextTextSurface);

                SDL_Surface *petitTextTextSurface = TTF_RenderText_Solid(font, numChar, petitTextColor);
                SDL_Texture *petitTextTexture = SDL_CreateTextureFromSurface(renderer, petitTextTextSurface);

                int petitTextTaille = 25;
                int grandTextTaille = 35;

                SDL_Rect grandTextRect = {
                    case_choix.x + (case_choix.w  - grandTextTaille) / 2,
                    case_choix.y + (case_choix.h / 2 - grandTextTaille) / 2,
                    grandTextTaille,
                    grandTextTaille
                };


                SDL_Rect petitTextRect = {
                    case_choix.x + (case_choix.w  - petitTextTaille) / 2,
                    case_choix.y + case_choix.h / 2 + (case_choix.h / 2 - petitTextTaille) / 2,
                    petitTextTaille,
                    petitTextTaille
                };

                SDL_RenderCopy(renderer, grandTextTexture, NULL, &grandTextRect);
                SDL_RenderCopy(renderer, petitTextTexture, NULL, &petitTextRect);
        }
}

// Changement de couleur
void changercouleur(SDL_Renderer *renderer, SDL_Rect caseClique) {
    SDL_SetRenderDrawColor(renderer, 170, 203, 255, 255); // couleur bleu
    SDL_RenderFillRect(renderer, &caseClique); // Dessiner un carreau
}

void reinitialiserGrilleCouleurs(SDL_Renderer *renderer, SDL_Rect **tab_cases) {
            for (int i = 0; i < TAILLE; i++) {
                for (int j = 0; j < TAILLE; j++) {
                      SDL_Rect tab_case = tab_cases[i][j];
                      // on remet la couleur initiale
                      SDL_SetRenderDrawColor(renderer, 245, 245, 220, 255); // Gris clair pour les carreaux
                      SDL_RenderFillRect(renderer, &tab_case);
                }
            }
}

int* recupererCase(SDL_Rect **tab_cases, int * caseClique, int position_x, int position_y) {
        int *coordonnes = malloc(2*sizeof(int));
        for (int i = 0; i < TAILLE; i++) {
                for (int j = 0; j < TAILLE; j++) {
                      SDL_Rect tab_case = tab_cases[i][j];
                      // on vérifie si le clique est à l'intérieur de la case
                      if(position_x >= tab_case.x && position_x <= tab_case.x + tab_case.w
                         && position_y >= tab_case.y && position_y <= tab_case.y + tab_case.h) {
                         coordonnes[0] = i;
                         coordonnes[1] = j;

                         return coordonnes;
                    }

                }
        }

        if(caseClique != NULL) {
            return caseClique;
        }

        return NULL;
}

int recupererCaseChoix(SDL_Rect *cases_choix, int position_x, int position_y) {
        for (int i = 0; i < TAILLE; i++) {
             SDL_Rect case_choix = cases_choix[i];
             if(position_x >= case_choix.x && position_x <= case_choix.x + case_choix.w
                         && position_y >= case_choix.y && position_y <= case_choix.y + case_choix.h)
                            return i + 1;
        }

        return 0;
}

// pour afficher les buttons effacer / help
void afficherButton(SDL_Renderer *renderer, TTF_Font *font, const char* text, SDL_Rect button) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &button);

    SDL_Color color = {0, 0, 255, 255}; // Couleur du texte (blanc)
    SDL_Surface* surface = TTF_RenderText_Solid(font, text, color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

    SDL_RenderCopy(renderer, texture, NULL, &button);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

// Fonction pour dessiner un bouton
void drawButton(SDL_Window *window, SDL_Renderer *renderer, TTF_Font *font, const char* text, int x, int y, int w, int h) {
    int width, height;
    SDL_GetWindowSize(window, &width, &height);

    SDL_Color color = {255, 255, 255, 255}; // Couleur du texte (blanc)
    SDL_Surface* surface = TTF_RenderText_Solid(font, text, color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

    SDL_Rect rect = {(width - w) / 2, y, w, h};
    SDL_RenderCopy(renderer, texture, NULL, &rect);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

// Fonction pour vérifier si la souris est sur un bouton
int isMouseOverButton(int mouseX, int mouseY, SDL_Window *window, int y, int w, int h) {
    int width, height;
    SDL_GetWindowSize(window, &width, &height);
    int x = (width - w) / 2;
    return mouseX >= x && mouseX <= x + w && mouseY >= y && mouseY <= y + h;
}

// Fonction pour afficher le contenu d'une partie
void displayPartContent(SDL_Window *window, SDL_Renderer *renderer, TTF_Font *font, const char* partName) {
    // Effacer l'écran
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Fond noir
    SDL_RenderClear(renderer);

    // Afficher le nom de la partie
    drawButton(window, renderer, font, partName, 350, 250, 100, 50);

    // Mettre à jour l'écran
    SDL_RenderPresent(renderer);

    // Attendre un peu pour que l'utilisateur voie le message
    SDL_Delay(2000); // 2 secondes
}

// Gestion du clic sur le bouton "HELP"
void fillEmptyCell() {

}

// Gestion du clic sur le bouton "HELP"
void handleSupprimerButtonClick(SDL_Window *window, SDL_Renderer *renderer, TTF_Font *font, SDL_Rect buttonEffacer, int position_x, int position_y, int selectedX,
     int selectedY, SDL_Rect **tab_cases, int grille[TAILLE][TAILLE], int caseModifie[2]) {

    if(position_x >= buttonEffacer.x && position_x <= buttonEffacer.x + buttonEffacer.w
                         && position_y >= buttonEffacer.y && position_y <= buttonEffacer.y + buttonEffacer.h) {
                eraseIncorrectCell(window, renderer, font, selectedX, selectedY, tab_cases, grille, caseModifie);
        }
}

void eraseIncorrectCell(SDL_Window *window, SDL_Renderer *renderer, TTF_Font *font, int selectedX, int selectedY,
        SDL_Rect **tab_cases, int grille[TAILLE][TAILLE], int caseModifie[2]) {
    printf("selectedX: %d\n", selectedX);
        printf("selectedY: %d\n", selectedY);

    if (selectedX != -1 && selectedY != -1) {
        grille[selectedX][selectedY] = 0; // Effacer la case
        caseModifie[0] = -1;
        caseModifie[1] = -1;
        tracerGrille(tab_cases, grille, renderer, window, font);
        tracerNumeros(tab_cases, grille, renderer, font);
    }
}
