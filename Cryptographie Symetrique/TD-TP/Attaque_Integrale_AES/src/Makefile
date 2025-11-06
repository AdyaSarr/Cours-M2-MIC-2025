CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g
TARGET = attaque
SOURCES = attaque_integrale.c aes.c
OBJECTS = $(SOURCES:.c=.o)

INCLUDES = -I../include 

# --- Règles de Compilation ---

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# --- Règles Utilitaires ---

clean:
	rm -f $(OBJECTS) $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run