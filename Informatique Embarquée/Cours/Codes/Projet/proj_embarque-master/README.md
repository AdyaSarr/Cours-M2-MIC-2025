"""README PROJET"""

Adya Sarr et Armand Jaulmes


Plan:

Commencer par installer les librairies et tout le bordel, notamment l'environnement python et initialiser toutes les variables. Faire des tests de base pour voir si tout fonctionne.

On pourra faire des structs pour les protocoles pour bien clarifier les échanges de données avec le code python.

Gestion de la réception des messages:
on fait un switch en fonction du message recu et on gère en fonction les erreurs et comportements.


Réfléchir rapidement au circuit, est-ce qu'on fait 2 circuits pour la led et le bouton?
Je préfère 1 circuit, on peut toujours wait() l'action du bouton pendant le clignotement de la LED et désactiver la broche lorsqu'on appuie sur le bouton. 
-------------------------------------------------

Erratum sur l'utilisation de la librairie micro-ecc:

utiliser #include "uECC.h"

Avec cette librairie, on peut choisir d'aller plus ou moins vite avec des codes de tailles variables.
Je pense qu'il est important de choisir ceux avec le code le plus petit possible. Bien sûr, on pourra comparer les temps s'ils sont vraiemnt beaucoup plus long.



On peut aussi choisir de compresser ou non nos clés pour optimiser la taille prise en mémoire.



#ifndef uECC_SUPPORTS_secp160r1
    #define uECC_SUPPORTS_secp160r1 1

    garder cette courbe à 1 comme indiqué dans le projet et mettre les autres à 0.

    On a une fonction qui prends en entrée notre fonction pseudo-aléatoire pour générer une clé aléatorement.


    Courbe utilisée:

    For secp160r1, private_key must be 21 bytes long! Note that the first byte will
                  almost always be 0 (there is about a 1 in 2^80 chance of it being non-zero).

    On a à priori 2 variables globales public_key et private_key qui seront modifiés par uECC_curve_public_key_size(ECC_Curve curve)

-----------------------------------------------------------------------------
Gestion de l'aléatoire:

    -Utiliser les valeurs du watchdog_timer à un moment aléatoire comme quand est-ce que la personne appuye sur le bouton(regarder combien de bits on peut réellement en tirer)

    -utiliser le signal électrique à un moment t pour générer des nombres selon l'intervalle où il est
    
-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
Partie 1: Enregistrement

1. Initialisation et Attente de la Requête (Client -> Authenticator)
    Préparation de l'environnement matériel et logiciel de l'Authenticator
    a. Initialiser la Communication Série (UART):
        -115 200 baud,
        -8 bits de données,
        -pas de parité,
        -1 bit de stop
    b. Initialiser les Périphériques:
        -Configurer la LED clignotante,
        -et le bouton de consentement.
    c. Attendre la Requête: L'Authenticator doit attendre de recevoir un message du Client.
        La requête MakeCredentialRequest commence par l'octet 0x01 qui est la valeur de COMMAND_MAKE_CREDENTIAL
    d. Format du Message MakeCredentialRequest
        -Octet 1 : 0x01 pour la commande MakeCredential.
        -Octets 2 à 21 : Empreinte SHA1(app_id) du Relying Party (20 octets), transférée en little-endian


2. Validation de la Requête et Gestion des Erreurs
    Cette phase se declenche immediatement apres la reception complete et l'identifiaction correcte du message MakeCredentialRequest
    a. Validation de la Requête
        -Critère de validation: Si lors de la reception ou de l'analyse du message des 21 octets, une erreur de lecture ou de formatage est detectée
        -Action requise: L'Authenticator doit immediatement envoyer un message d'erreur au client code d'erreur: STATUS_ERR_BAD_PARAMETER 3
    b. Demande de Consentement de l'Utilisateur
        Si la requete est alide, l'authenticator doit interagir avec l'utilisateur pour obtenir son accord
        -Indication Visuelle:
            --Action: L'Authenticator doit indiquer a l'utilisateur qu'il doit donner son consentement
            --Moyen: Faire clignoter la Led a un intervalle de 0,5 seconde (fréquence de 1 Hz).
        -Obtention du Consenteme
            --Action: L'utilisateur donne son consentement en appuyant sur un bouton relié a l'Authenticateur
            --Conséquence: L'appui sur le boutton doit etre arreter le clignotement de la LED et permettre de passer a l'etape suivante
        -Gestion du Délai d'Attente (Timeout)
            L'authencator ne doit pas attendre indéfiniment le consentement:
            --Timeout: L'utilisateur n'a pas donné son consentement au bout de 10 secondes, le processus doit etre annulé
            --Action d'Erreur: Un message d'erreur doit être renvoyé au Client code d'erreur: STATUS_ERR_APPROVAL 6


3. Génération et Stockage des Clés
    Cette partie est constituée de trois operations consecutives:
    
    a. Génération de la Paire de Clés ECDSA:
        L'authenticator doit creer une paire de clés asymetrique propre a l'application(Replying Party ou RP)
        -Algorithme: ECDSA
        -Courbe: secp160r1, on l'utilise pour son temps de calcul acceptable sur le microcontrôleur ATmega328P
        -Outil: utlisation de la bibliothèque micro-ecc pour implémenter cet algorithme.
        -Tailles des Clés:
            --Clé Privée :21 octets.
            --Clé Publique: 40 octets.
        -Source d'Aléatoire:
            La génération de clés nécessite une source d'aléatoire cryptographiquement sûre (TRNG).
            --Contrainte: L'ATmega328P ne possède pas de générateur d'aléatoire matériel.
            --Solution: Convertisseur Analogique-Numérique (ADC)
        -Code d'Erreur: STATUS_ERR_CRYPTO_FAILED 2 : si la génération de la paire de clés échoue
    b. Génération de l'Identifiant de Clé (credential_id)
        Une fois la paire de clés générée avec succés l'Authenticator crée un identifiant unique associé à cette paire
        -Nom : credential_id
        -Longueur : 128 bits (soit 16 octets).
        -Contrainte: Cet identifiant ne doit pas être lié au nombre de paires de clés stockées par l'Authenticator. Il doit être généré aléatoirement ou de manière unique.
    c. Stockage en Mémoire Non Volatile:
        Le couple (Clé privée, credential_id) doit être conservé de façon persistante dans la mémoire non volatile (EEPROM) de l'Authenticator, associé à l'application.
        -Association Stockée: L'Authenticator sauvegarde le triplet suivant:
            Association = (SHA1(app_id), credential_id, Clé Privée)
        -Remplacement: Si une entrée existe déjà pour le même SHA1(app_id), ses valeurs précédentes doivent être remplacées par la nouvelle entrée.
        -Code d'erreur de stockage: STATUS_ERR_STORAGE_FULL 5: si la mémoire non volatile est saturée et qu'il n'est pas possible d'ajouter la nouvelle entrée
    d. Envoi de la Réponse de Succès:
        Si toutes les étapes (génération de clé, identifiant et stockage) réussissent sans erreur, l'Authenticator envoie la réponse de succès au Client.
        -Message: MakeCredentialResponse
        -Format: Le message commence par STATUS_OK (0x00) et contient ensuite:
            --Le credential_id (16 octets).
            --La clé publique générée (40 octets).
    e. Définition de la Structure de Stockage
        -Taille de l'EEPROM:  L'ATmega328P possède 1024 octets d'EEPROM
        -Taille d'une Entrée: Une entrée complète doit contenir l'empreinte de l'application, l'identifiant, et la clé privée :
            --SHA1(app_id) : 20 octets
            --credential_id : 16 octets
            --Clé privée : 21 octets
            --Total par entrée : 20 + 16 + 21 = 57 octets.
        -Capacité Maximale : 1024 / 57 = 17 entrées.
-----------------------------------------------------------------------------
Partie 2: Authentification

• MakeCredential : utilisé lors de l’enregistrement de l’Authenticator auprès d’un Relying
Party


Partie 2: Connexion

• GetAssertion : utilisé lors d’une authentification à l’aide de l’Authenticator auprès d’un
Relying Party


2 autres types de fonctions utilisables à n'importe quel moment:

• ListCredentials : permet de lister les Relying Parties enregistrés dans l’Authenticator (ainsi
que l’identifiant unique de la clé qui leur est associé)
• Reset : permet de réinitialiser la mémoire non volatile de l’Authenticator et de supprimer
toutes les clés existantes

----------------------------------------------------------------------------------------
Quand et comment dormir?: (réétudier potentiellement quel sleep mettre mais normaelemnt le même que tp3)

    Là où il FAUT dormir:
        -quand on a envoyé une requête par le client et qu'on attend la réponse. On sleep soit jusqu'à la réception du message soit jusqu'à l'interruptiion timer (trop d'attente).
    On activer un IRS pour se réveiller lors de la réception du message.
        -pendant les intervalles de clignotements de la LED
        -entre le moment où on recoit la réponse et le moment où l'utilisateur appuie sur le bouton

        -après avoir envoyé la signature au client


------------------------------------------------------------------------------------------------------------
Gestion de la mémoire:
    -Réinitialiser les variables qu'on a modifié avec Blink_LED
    -Credential_id clairement en mémoire volatile, puis à posteriori la stocker après avoir effectué les calculs pour le tuple.

    -Sur la partie Authentification, ne rien garder (tout volatile)



    