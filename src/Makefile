CC 		:= gcc
CFLAGS	:= -Wall -Wextra -g `llvm-config --cflags` -I. -Icodegen -fsanitize=address  
LDFLAGS	:= `llvm-config --libs core` -fsanitize=address 

SRC := main.c lexer.c parser.c ast.c linkedlist.c codegen/codegen.c codegen/codegen_statement.c codegen/codegen_expression.c codegen/codegen_type.c codegen/codegen_subprogram.c symboltable.c types.c builtins.c 

TARGET := frascal

all: $(TARGET)

parser.c: parser.y
	bison -d -g -o $@ $^

parser.h: parser.c

lexer.c: lexer.l 
	flex -o $@ $^

$(TARGET): $(SRC) 
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@
	@echo "Finished compiling $(TARGET)"


# disable address sanitize flag to use this
.PHONY: memcheck
memcheck: 
	valgrind --leak-check=full --show-leak-kinds=all ./$(TARGET) test.frp

.PHONY: test 
test: $(TARGET) 
	./$(TARGET) test.frp 
	llc -relocation-model=pic out.ll -o out.s
	gcc out.s -fPIE -pie -o test
	./test

.PHONY: clean
clean : 
	rm -rf lexer.c parser.c parser.h $(TARGET) parser.gv parser.png out.ll a.out out.s test
