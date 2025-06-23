# Compilador e flags
CC = gcc
CFLAGS = -Iinclude -Wall -g

# Pastas
SRC_DIR = src
BUILD_DIR = build
INCLUDE_DIR = include

# Arquivos
TARGET = $(BUILD_DIR)/cshort
OBJS = $(BUILD_DIR)/lexer.o $(BUILD_DIR)/parser.o $(BUILD_DIR)/symbols.o $(BUILD_DIR)/main.o

# Regra principal
all: $(TARGET)

# Cria o executável
$(TARGET): $(OBJS)
	$(CC) -o $@ $^

# Compila lexer.c
$(BUILD_DIR)/lexer.o: $(SRC_DIR)/lexer.c $(INCLUDE_DIR)/lexer.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Compila parser.c
$(BUILD_DIR)/parser.o: $(SRC_DIR)/parser.c $(INCLUDE_DIR)/parser.h $(INCLUDE_DIR)/lexer.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Compila main.c
$(BUILD_DIR)/main.o: $(SRC_DIR)/main.c $(INCLUDE_DIR)/lexer.h $(INCLUDE_DIR)/parser.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Tabela de Simbolos 
$(BUILD_DIR)/symbols.o: $(SRC_DIR)/symbols.c $(INCLUDE_DIR)/symbols.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Cria o diretório build/ se não existir
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Limpa os arquivos compilados
clean:
	rm -f $(BUILD_DIR)/*.o $(TARGET)

.PHONY: all clean
