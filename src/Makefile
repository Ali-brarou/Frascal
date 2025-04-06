CC 		= gcc
CFLAGS 	= -Wall -Wextra -g `llvm-config --cflags --libs core` -fsanitize=address

all: frascal

parser.c: parser.y
	bison -d -g -o $@ $^

parser.h: parser.c

lexer.c: lexer.l 
	flex -o $@ $^

frascal: main.c lexer.c parser.c ast.c linkedlist.c codegen.c symboltable.c
	$(CC) main.c lexer.c parser.c ast.c linkedlist.c codegen.c symboltable.c $(CFLAGS) -o $@


.PHONY: memcheck
memcheck: 
	valgrind --leak-check=full --show-leak-kinds=all ./frascal test.frp

.PHONY: clean
clean : 
	rm -rf lexer.c parser.c parser.h frascal parser.gv parser.png
