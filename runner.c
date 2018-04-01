#include <stdio.h>
#include "symbolTable.h"

extern int yyparse();
extern void yyset_in(FILE *);

FILE *inputFile;

int main(int argc, char* argv[])
{
    // "symbols" global var is defined in symbolTable.h
    // It mainins the pointer to the current symbol table entry
    // for use by yacc/bison
   
    if (argc == 1) {
        printf("No program to compile\n");
        return 1;
    }
    else {
        const char* program = argv[1];
        inputFile = fopen(program, "r");
        yyset_in(inputFile);
        printf("yyparse exit code: %d\n", yyparse());

        return 0;
    }

    return 0;
}