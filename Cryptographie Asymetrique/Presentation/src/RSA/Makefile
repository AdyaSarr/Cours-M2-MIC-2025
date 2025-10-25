# --- Variables de Compilation ---
# Détecter le chemin de GMP si Homebrew est utilisé (Mac/Linux)
# On utilise le chemin standard pour Homebrew sur les Macs récents (Apple Silicon)
GMP_PREFIX := /opt/homebrew/opt/gmp

# Flags de compilation C standard
CC = gcc
CFLAGS = -Wall -Wpointer-arith -std=c99 -g

# Flags d'inclusion et de liaison GMP
# -I : Inclut l'en-tête (gmp.h)
# -L : Spécifie le chemin de la bibliothèque (libgmp.a)
# -l : Lie la bibliothèque (libgmp)
GMP_INC = -I$(GMP_PREFIX)/include
GMP_LIB = -L$(GMP_PREFIX)/lib -lgmp

# Exécutable final
TARGET = rsa

# --- Cibles par Défaut (Compilation) ---

# Règle par défaut : construire l'exécutable
all: $(TARGET)

$(TARGET): rsa.c rsa.h
	$(CC) $(CFLAGS) $(GMP_INC) rsa.c -o $(TARGET) $(GMP_LIB)
	@echo "========================================================"
	@echo "Compilation réussie. Exécutez 'make test' pour les modes."
	@echo "========================================================"

# --- Cibles d'Automatisation et de Test ---

# Nettoie les fichiers temporaires, clés et chiffrés
clean:
	@echo "Nettoyage des fichiers générés..."
	rm -f $(TARGET) *.pem *.txt *.o
	@echo "Nettoyage terminé."

# Cible pour exécuter la séquence de tests montrée dans le terminal
test: all
	@echo "========================================================"
	@echo "DÉBUT DE LA SÉQUENCE DE TEST RSA AUTOMATISÉE"
	@echo "========================================================"
	
	# Créer un fichier message factice (si le programme le lit)
	@echo "Hello World! C'est un message simple pour RSA." > message.txt
	
	# --- Séquence 1: Génération, Chiffrement, et Déchiffrement ---
	@echo "--- 1. MODE 1 : GÉNÉRATION & CHIFFREMENT (Nouvelles Clés) ---"
	./$(TARGET) message.txt
	@echo ""

	@echo "--- 2. MODE 3 : CHIFFREMENT (Clé Publique existante) ---"
	./$(TARGET) message.txt pub.pem
	@echo ""

	@echo "--- 3. MODE 2 : DÉCHIFFREMENT (Clé Privée de cypher1.txt) ---"
	./$(TARGET) cypher1.txt priv.pem
	@echo ""
	
	@echo "--- 4. MODE 2 : DÉCHIFFREMENT (Clé Privée de cypher.txt) ---"
	./$(TARGET) cypher.txt priv.pem
	@echo ""

	@echo "========================================================"
	@echo "SÉQUENCE DE TEST TERMINÉE."
	@echo "Vérifiez les fichiers *.pem et *.txt."
	@echo "========================================================"

# Spécifier les cibles qui ne correspondent pas à des noms de fichiers
.PHONY: all clean test