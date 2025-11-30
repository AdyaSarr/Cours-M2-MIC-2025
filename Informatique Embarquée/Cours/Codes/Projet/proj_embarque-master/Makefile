MCU = atmega328p
F_CPU = 16000000UL
TARGET = authenticator
AVR_PORT = /dev/tty.usbmodem*



SRC_MAIN = enregistrement.c

# Dossier contenant la librairie micro-ecc
UECC_DIR = micro-ecc-master

# Fichiers source de micro-ecc nécessaires
# uECC.c pour la source crypto, et asm_avr.inc pour les optimisations AVR
SRC_UECC = $(UECC_DIR)/uECC.c $(UECC_DIR)/asm_avr_mult_square.inc $(UECC_DIR)/asm_avr.inc

#Tous les fichiers sources
SRCS = $(SRC_MAIN) $(UECC_DIR)/uECC.c
SRC_UECC_C = $(UECC_DIR)/uECC.c

# Liste de tous les fichiers objets (.o) à compiler
OBJS = $(SRC_MAIN:.c=.o) $(SRC_UECC_C:.c=.o)


CC = avr-gcc
OBJCOPY = avr-objcopy
AVRDUDE = avrdude

CFLAGS = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -Os -Wall -I. -I$(UECC_DIR)
# Flags d'assemblage (pour les fichiers .inc de micro-ecc)
ASFLAGS = -mmcu=$(MCU)

# Flags d'édition de liens: inclusion des librairies C standard et AVR
LDFLAGS = -mmcu=$(MCU)

all: $(TARGET).hex

# Regle pour lier tous les fichiers objets et créer l'ELF
$(TARGET).elf: $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@
#Règle de compilation des fichiers C
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Règle pour les fichiers d'assemblage de micro-ecc (.inc)
$(UECC_DIR)/uECC.o: $(UECC_DIR)/uECC.c
	$(CC) $(CFLAGS) -c $< -o $@


# Création du fichier HEX pour le téléversement
$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@
	$(OBJCOPY) -j .eeprom --set-section-flags=.eeprom="alloc,load" --change-section-lma .eeprom=0 -O ihex $< $(TARGET).eep

# Règle de téléversement (Utilise le port générique pour macOS)
# Remplace "arduino" par "usbasp" ou autre si on n'utilise pas le bootloader Arduino
program: $(TARGET).hex
	$(AVRDUDE) -p $(MCU) -c arduino -P $(AVR_PORT) -b 115200 -U flash:w:$(TARGET).hex

clean:
	rm -f $(TARGET).elf $(TARGET).hex $(TARGET).eep *.o $(UECC_DIR)/*.o
	# Nettoyer également les objets de la librairie externe
	rm -f $(UECC_DIR)/uECC.o
	rm -f $(UECC_DIR)/asm_avr_mult_square.o

.PHONY: all program clean