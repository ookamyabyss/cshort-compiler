CC = gcc
CFLAGS = -Iinclude -Wall -g

build/lexer: build/lexer.o build/main.o
	$(CC) -o build/lexer build/lexer.o build/main.o

build/lexer.o: src/lexer.c include/lexer.h
	$(CC) $(CFLAGS) -c src/lexer.c -o build/lexer.o

build/main.o: src/main.c include/lexer.h
	$(CC) $(CFLAGS) -c src/main.c -o build/main.o

clean:
	rm -f build/*.o build/lexer
