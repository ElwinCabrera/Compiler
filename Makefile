#Source files
SRC= runner.c symbol_table.c \
 types.c stack.c assignable.c \
 address.c intermediate_code.c \
 expression.c linked_list.c \
 code_blocks.c registers.c \
 assembly.c data_block.c \
 location.c graph.c

TEST_SRC=$(filter-out runner.c, $(SRC))

TESTS= cunit/tests.c \
	cunit/symbol_table_tests.c
	
#Autogenerated files
GEN=lex.yy.c y.tab.c y.tab.h

#Additional dependencies
DEP=					

# OS identification from: https://stackoverflow.com/questions/714100/os-detecting-makefile
OS := $(shell uname -s)

ifeq ($(OS), Darwin) 
  CUNIT_PATH_PREFIX = /usr/local/Cellar/cunit/2.1-3/
  CUNIT_DIRECTORY = cunit
endif

ifeq ($(OS), Linux) 
  CUNIT_PATH_PREFIX = /util/CUnit/
  CUNIT_DIRECTORY = CUnit/
endif

ifeq ($(OS), CYGWIN_NT-10.0) 
  CUNIT_PATH_PREFIX = /usr/
  CUNIT_DIRECTORY = CUnit/
endif

all: compiler_tools $(SRC)
	gcc -g -o compiler $(SRC) $(GEN) -Wall

# No debug flags as per submission guidelines
compiler: compiler_tools $(SRC)
	gcc -o compiler $(SRC) $(GEN) -Wall

compiler_tools: syntax_analayzer lexical_analyzer

lexical_analyzer: lexical_structure.lex y.tab.h
	flex lexical_structure.lex

syntax_analayzer: grammar.y
	bison -d -v -y grammar.y

tests: $(TEST_SRC) $(GEN) $(TESTS)
	$(CC) $(CFLAGS) -lm -L $(CUNIT_PATH_PREFIX)lib -I $(CUNIT_PATH_PREFIX)include/$(CUNIT_DIRECTORY) -g $^ -o tests -lcunit -lgcov 

clean:
	rm -f $(GEN) compiler tests y.output
