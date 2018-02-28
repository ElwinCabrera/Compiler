
#Any .c file
SRC=$(wildcard *.c) 				
#Any .o file
OBJ=$(SRC:.c=.o)	
#Any additional .h files
DEP=						

all: precompile $(OBJ) $(DEP)
	gcc -o compiler $(OBJ) -Wall

%.o: precompile S(SRC)
	gcc -o $@ $< -Wall

precompile: lexical_analyzer #grammar

#grammar: grammar.y

lexical_analyzer: lexicalStructure.lex
	flex lexicalStructure.lex

clean:
	rm -f $(OBJ) compiler

.PHONY: all clean