#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include "operations_binaires.h"


char *addition_binaire(const char *entier1, const char *entier2){
    size_t len_entier1 = strlen(entier1);// contient la taille de la chaine
    size_t len_entier2 = strlen(entier2);//idem

    size_t len_resultat = (len_entier1>=len_entier2? len_entier1 + 2: len_entier2 +2);// comme je sais que la taille d'addition est au plus max(len_entier1, len_entier2)+1 et l'autre +1 pour le caractere de fin
    char *resultat_add;
    resultat_add = calloc(len_resultat, sizeof(char));//j'alloue une taille qui pourra contenir tout le resultat de l'addition
    if (!resultat_add)
    {
        fprintf(stderr, "calloc: probleme d'allocation sur l' addition\n");
        return NULL;
    }
    
    long i = len_entier1 -1;//pour parcourir tous les bits de l'entier1 sauf le caractere '/0'
    long j = len_entier2 -1;// idem mais pour l'entier2
    long k = len_resultat-2;//pour le resultat

    int retenue = 0;

    while (i>=0 || j>=0 || retenue !=0)//pour voir s'il ya des elments a parcourir sur les deux chaines ou si on parcourt toutes les deux chaines mais que la retenu est 1 on fait l'addition
    {
        int val_entier1 = (i>=0? entier1[i]-'0': 0); //Convertir l'element i de la chaine en entier
        int val_entier2 = (j>=0? entier2[j]-'0':0);//idem

        int somme = val_entier1 + val_entier2 + retenue;
        
        retenue = somme/2;

        resultat_add[k]=somme%2 + '0';
        i--;
        j--;
        k--;
    }
    
    // Nettoyage des zéros non significatifs (leading zeros)
    //size_t start = 0;
    
    // Déterminer le début du résultat (start)
    // k+1 est l'indice où le résultat commence dans la mémoire allouée
    char *resultat_debut = resultat_add + (k + 1);

    // Trouver le premier '1' ou s'arrêter juste avant le terminateur
    while (*resultat_debut == '0' && *(resultat_debut + 1) != '\0') {
        resultat_debut++;
    }
    
    // Si la chaîne est complètement '0', on retourne "0"
    if (*resultat_debut == '\0' && (resultat_add + k + 1) != resultat_debut) {
        // Cas où la chaîne entière était 000...0
        // On retourne la chaîne '0' allouée dynamiquement pour être cohérent
        resultat_add[0] = '0';
        resultat_add[1] = '\0';
        return resultat_add;
    }

    // Déplacement de la partie significative au début de l'espace alloué
    size_t len_reel = strlen(resultat_debut);
    memmove(resultat_add, resultat_debut, len_reel + 1); // +1 pour le '\0'
    
    return resultat_add;
}


char *complement_a_deux(const char *entier, size_t ovaleaf){
    if (!entier)
    {
        return NULL;
    }
    
    size_t len_entier = strlen(entier);
    char *copie = calloc(ovaleaf+1, sizeof(char));

    if (!copie)
    {
        fprintf(stderr, "calloc: probleme sur l'allocation dans complement a deux\n");
        return NULL;
    }
    
    if (len_entier>ovaleaf)
    {
        memcpy(copie, entier+(len_entier-ovaleaf), ovaleaf);
    }else
    {
        memset(copie, '0', ovaleaf-len_entier);
        memcpy(copie + (ovaleaf-len_entier), entier, len_entier);
    }
    

    for (size_t i = 0; i < ovaleaf; i++) {
        copie[i] = (copie[i] == '0') ? '1' : '0';
    }
    copie[ovaleaf]= '\0';
    char *somme = addition_binaire(copie, "1");
    free(copie);
    if (!somme)
    {
        fprintf(stderr, "Addition: sur la soustraction complement a deux\n");
        return NULL;
    }
    //Conserver ovaleaf bit
    size_t L = strlen(somme);
    const char *src = (L > ovaleaf) ? (somme + (L - ovaleaf)) : somme;

    char *out = malloc(ovaleaf+1);
    if (!out)
    {
        fprintf(stderr, "malloc: complement a deux\n");
        free(somme);
        return NULL;
    }

    if (L < ovaleaf)
    {
        memset(out, '0', ovaleaf - L);
        memcpy(out+(ovaleaf-L), somme, L);
    }else
    {
        memcpy(out, src, ovaleaf);
    }
    out[ovaleaf] = '\0';
    free(somme);
    return out;
}


char *padding_left(char *entier, size_t ovaleaf){
    size_t L = strlen(entier);
    const char *src = (L > ovaleaf) ? (entier + (L - ovaleaf)) : entier;

    char *out = malloc(ovaleaf+1);
    if (!out)
    {
        fprintf(stderr, "malloc: complement a deux\n");
        return NULL;
    }

    if (L < ovaleaf)
    {
        memset(out, '0', ovaleaf - L);
        memcpy(out+(ovaleaf-L), entier, L);
    }else
    {
        memcpy(out, src, ovaleaf);
    }
    out[ovaleaf] = '\0';
    return out;
}

char *soustraction_avec_cpmt_deux(char *entier1,char *entier2){
    size_t len_entier1 = strlen(entier1);
    size_t len_entier2 = strlen(entier2);
    size_t ovaleaf = (len_entier1>=len_entier2? len_entier1: len_entier2);

    //faire du padding vers la gauche des deux entiers
    char *A = padding_left(entier1, ovaleaf);
    if (!A)
    {
        fprintf(stderr, "Padding: sur la soustraction avec complement a deux\n");
        return NULL;
    }
    char *B = padding_left(entier2, ovaleaf);
    if (!B)
    {
        fprintf(stderr, "Padding: sur la soustraction avec complement a deux\n");
        free(A);
        return NULL;
    }
    
    char *complement_B = complement_a_deux(B,ovaleaf);
    if (!complement_B)
    {
        fprintf(stderr, "Complement a deux: sur la soustraction\n");
        free(A);
        free(B);
        return NULL;
    }
    char *resultat_sub = addition_binaire(A, complement_B);
    free(A);
    free(B);
    free(complement_B);
    if (!resultat_sub)
    {
        fprintf(stderr, "Addition: sur la soustraction avec complement a deux\n");
        return NULL;
    }
    char *out = padding_left(resultat_sub, ovaleaf);
    free(resultat_sub);
    if (!out)
    {
        fprintf(stderr, "Padding: Erreur sur le resultat\n");
        return NULL;
    }
    
    return out;
}

char *soustraction_naive(const char *entier1, const char *entier2){
    size_t len_entier1 = strlen(entier1);
    size_t len_entier2 = strlen(entier2);

    size_t len_resultat = (len_entier1>=len_entier2? len_entier1+2: len_entier2+2);

    char *resultat = calloc(len_resultat, sizeof(char));

    if (!resultat)
    {
        fprintf(stderr, "calloc: sur la soustraction naive\n");
        return NULL;
    }

    long i = len_entier1 - 1;
    long j = len_entier2 - 1;
    long k = len_resultat - 2;
    
    int retenue = 0;

    while (i>=0 || j>=0)
    {
        int val_entier1 = (i>=0? entier1[i] - '0': 0);
        int val_entier2 = (j>=0? entier2[j] - '0': 0);

        int difference = val_entier1 - val_entier2 - retenue;

        if (difference < 0)
        {
            retenue = 1;
            difference += 2;
        }else
        {
            retenue = 0;
        }
        resultat[k] = difference + '0';
        i--;
        j--;
        k--;
    }
    if (retenue) {
        fprintf(stderr, "soustraction_naive: entier1 < entier2 (non signe non supporte)\n");
        free(resultat);
        return NULL;
    }

    // Nettoyage des zéros non significatifs (Corrigé et rendu sûr)
    char *resultat_debut = resultat + (k + 1);

    while (*resultat_debut == '0' && *(resultat_debut + 1) != '\0') {
        resultat_debut++;
    }

    size_t len_reel = strlen(resultat_debut);
    // Cas où le résultat est 000...0 ou 0
    if (len_reel == 0 && *resultat_debut == '\0' && (resultat + k + 1) != resultat_debut) {
        resultat[0] = '0';
        resultat[1] = '\0';
        return resultat;
    }
    
    memmove(resultat, resultat_debut, len_reel + 1);
    return resultat;
}


char *multiplication_naive_binaire(const char *entier1, const char *entier2) {
    size_t len1 = strlen(entier1);
    size_t len2 = strlen(entier2);
    if (len1 == 0 || len2 == 0) return strdup("0");

    
    size_t max_len = len1 + len2;
    
    // Initialiser le résultat à "0"
    // On alloue max_len + 1 pour l'espace de travail du décalage
    char *resultat = calloc(max_len + 1, sizeof(char)); 
    if (!resultat) return NULL;
    resultat[0] = '0'; // Le résultat initial est "0"
    
    // Pointeur pour contenir le résultat de l'addition à chaque étape
    char *somme_courante = resultat; // Initialisé à '0'

    // Boucle sur les bits de entier2 (de droite à gauche)
    for (int j = len2 - 1; j >= 0; j--) {
        int bit = entier2[j] - '0';
        
        // Décalage pour l'étape actuelle
        // La multiplication par 2^m est faite en ajoutant 'j' zéros à droite.
        // Puisque nous décalons de droite à gauche (j = len2-1 -> 0), 
        // le décalage pour l'itération 'j' est de (len2 - 1 - j) zéros.
        
        size_t decalage_zeros = len2 - 1 - j;
        
        if (bit == 1) {
            // Créer le multiplicande décalé : entier1 + zéros
            char *terme_decalage = calloc(len1 + decalage_zeros + 1, sizeof(char));
            if (!terme_decalage) {
                free(resultat);
                return NULL;
            }
            
            // Copier entier1 au début
            strcpy(terme_decalage, entier1);
            
            // Ajouter les zéros (caractères '0') à la fin
            for (size_t z = 0; z < decalage_zeros; z++) {
                terme_decalage[len1 + z] = '0';
            }
            terme_decalage[len1 + decalage_zeros] = '\0';

            // Additionner le terme décalé au résultat courant
            // addition_binaire est censée gérer la taille et les carrys
            char *nouveau_resultat = addition_binaire(somme_courante, terme_decalage);
            
            // Nettoyage de l'ancien résultat et du terme temporaire
            free(somme_courante);
            free(terme_decalage);
            
            if (!nouveau_resultat) return NULL;
            somme_courante = nouveau_resultat; 
        }
        
        // Si le bit est 0, on n'ajoute rien, on passe à l'itération suivante.
    }

    // Le résultat doit être dépouillé des zéros non significatifs (padding_right)
    return somme_courante; 
}

split_ent split_entier(int entier, int k){
    split_ent entiers = {0, 0};

    //int k = (sizeof(unsigned int) * CHAR_BIT) - __builtin_clz(entier);
    int m = k/2;
    int mask = (1 << m) - 1;
    entiers.a_2 = entier & mask;
    entiers.a_1 = entier >> m;
    return entiers;
}
int karatsuba(int entier1, int entier2){
    if (entier1 == 0 || entier2 == 0)
    {
        return 0;
    }
    if (entier1==1)
    {
        return entier2;
    }
    if (entier2==1)
    {
        return entier1;
    }
    
    
    int len_entier1 = (sizeof(unsigned int) * CHAR_BIT) - __builtin_clz(entier1);
    int len_entier2 = (sizeof(unsigned int) * CHAR_BIT) - __builtin_clz(entier2);

    int k = (len_entier1 > len_entier2? len_entier1:len_entier2);
    if (k==1)
    {
        return entier1*entier2;
    }
    int m=k/2;

    split_ent entiers_entier1 = split_entier(entier1, k);
    split_ent entiers_entier2 = split_entier(entier2, k);

    int a1b1 = karatsuba(entiers_entier1.a_1, entiers_entier2.a_1);
    int a2b2 = karatsuba(entiers_entier1.a_2, entiers_entier2.a_2);

    int somme_enter1 = entiers_entier1.a_1+entiers_entier1.a_2;
    int somme_entier2 = entiers_entier2.a_1+entiers_entier2.a_2;
    int terme_facteur = karatsuba(somme_enter1, somme_entier2);
    int terme_du_milieu = terme_facteur - a1b1 - a2b2;
    return (a1b1 <<(2*m)) + (terme_du_milieu << m) + a2b2;
}




int algo_Euclide(const int entier1,const int entier2){
    if (entier1==0)
    {
        return entier2;
    }
    if (entier2 == 0)
    {
        return entier1;
    }
    int max = (entier1>entier2? entier1:entier2);
    int min;
    if (max == entier1)
    {
        min = entier2;
    }else
    {
        min = entier1;
    }
    int reste = max%min;
    return algo_Euclide(min, reste);
}

int algo_Euclide_accelerer(int entier1, int entier2){
    if (entier1==0)
    {
        return entier2;
    }
    if (entier2 == 0)
    {
        return entier1;
    }
    int max = (entier1>entier2? entier1:entier2);
    int min;
    if (max == entier1)
    {
        min = entier2;
    }else
    {
        min = entier1;
    }
    int reste_1 = max%min;
    int reste_2 = min - reste_1;
    if (reste_1>reste_2)
    {
        return algo_Euclide_accelerer(min, reste_2);
    }
    return algo_Euclide_accelerer(min, reste_1);
}


params_euclide_etendu *euclide_etendu(int entier1, int entier2){
    params_euclide_etendu *resultat = malloc(sizeof(params_euclide_etendu));
    if (!resultat)
    {
        fprintf(stderr, "malloc: sur l'allocation des parametres d'euclide etendu\n");
        return NULL;
    }
    int max = (entier1>entier2? entier1:entier2);
    int min;
    if (max == entier1)
    {
        min = entier2;
    }else
    {
        min = entier1;
    }
    int quotien = 0;
    int reste = 0;
    int x_0 = 1;
    int x_1 = 0;
    int y_0 = 0;
    int y_1 = 1;
    int x_2, y_2; 
    do
    {
        quotien = max/min;
        reste = max - min*quotien;
        x_2 = x_0 - x_1*quotien;
        y_2 = y_0 - y_1*quotien;
        
        max = min;
        min = reste;

        x_0 = x_1;
        x_1 = x_2;

        y_0 = y_1;
        y_1 = y_2;
    } while (reste!=0);
    resultat->pgcd = max;
    resultat->x = x_0;
    resultat->y = y_0;

    return resultat;
}
void affichage_param_euclide(params_euclide_etendu *resultat, const int a, const int b){
    printf("Le theoreme de fermat appliqué a %d et %d\n", a,b);
    fprintf(stdout, "%d x %d + %d x(%d) = %d\n", a, resultat->x, b, resultat->y, resultat->pgcd);
}


int inverse_mod(const int entier,const int mod){
    int inv = 0;
    params_euclide_etendu *result = euclide_etendu(entier, mod);
    if (!result)
    {
        fprintf(stderr, "probleme d'allocation sur la fonction inverse_mod\n");
        return -1;
    }

    if (result->pgcd != 1)
    {
        return -1;
    }
    inv = result->x;
    free(result);
    return inv;
}

int main(int argc, char const *argv[])
{
    //declarartion et initialisation des variable de test
    char *entier1 = "1111";//"10101","10001"
    char *entier2 = "1111";//"1110","111"
    char *entier = "00011";
    char *entier3 = "000111";
    char *entier4 = "1011";
    char *entier5 = "1101";
    const int a = 252;//143
    const int b = 198;//23
    const int a1 = 143;
    const int b1 = 23;
    const int a2 = 17;
    const int b2 = 43;

    //Appel aux fonctions
    char *resultat_add = addition_binaire(entier1, entier2);
    char *complement_entier = complement_a_deux(entier,6);
    char *soustraction_sub = soustraction_avec_cpmt_deux(entier3, entier);
    char *soustraction_na = soustraction_naive(entier3, entier);
    int pgcd = algo_Euclide(a,b);
    int pgcd_ = algo_Euclide_accelerer(a1, b1);
    params_euclide_etendu *resultat_etendu = euclide_etendu(a, b);
    params_euclide_etendu *result = euclide_etendu(a2, b2);
    int inv = inverse_mod(17, 43);
    int mult_karat = karatsuba(a,b);
    char *result_mult_naive = multiplication_naive_binaire(entier4, entier5);

    //Les tests pour les erreurs
    if (!resultat_add)
    {
        fprintf(stderr, "Addtion: Erreur de l'addition\n");
        return 1;
    }
    if (!complement_entier)
    {
        fprintf(stderr, "Complement a deux: Erreur de complement a deux\n");
        return 1;
    }
    if (!soustraction_sub)
    {
        fprintf(stderr, "Soustraction: avec cpmt a deux");
        return 1;
    }
    if (!soustraction_na)
    {
        fprintf(stderr, "Soustraction: soustraction naive");
        return 1;
    }
    if (!resultat_etendu)
    {
        fprintf(stderr, "Euclide Etendu\n");
        return 1;
    }
    if (!result)
    {
        fprintf(stderr, "Euclide Etendu\n");
        return 1;
    }
    if (!result_mult_naive)
    {
        fprintf(stderr, "Multiplication niave\n");
        return 1;
    }
    
    
    //Affichage des sorties
    fprintf(stdout, "Addition: %s + %s = %s\n", entier1, entier2, resultat_add);
    fprintf(stdout, "Complement a deux: de %s est %s\n", entier, complement_entier);
    fprintf(stdout, "Soustraction avec complement a deux: %s - %s = %s\n", entier3, entier, soustraction_sub);
    fprintf(stdout, "Soustraction naive: %s - %s = %s\n", entier3, entier, soustraction_na);
    fprintf(stdout, "PGCD(%d, %d) = %d\n", a, b, pgcd);
    fprintf(stdout, "Accelerer PGCD(%d, %d) = %d\n", a, b, pgcd_);
    affichage_param_euclide(resultat_etendu, a, b);
    affichage_param_euclide(result, a2, b2);
    if (inv == -1)
    {
        fprintf(stdout, "L'inverse %d mod %d n'existe pas\n", a2, b2);
    }else
    {
        fprintf(stdout, "L'inverse de %d mod %d = %d\n", a2, b2, inv);
    }
    fprintf(stdout, "La multiplication karatsuba: %d x %d = %d\n", a, b, mult_karat);
    fprintf(stdout, "La multiplication naive: %s x %s = %s\n", entier4, entier5, result_mult_naive);
    

    //Nettoyage totale 
    free(resultat_add);
    free(complement_entier);
    free(soustraction_sub);
    free(soustraction_na);
    free(resultat_etendu);
    free(result);
    free(result_mult_naive);
    return 0;
}
