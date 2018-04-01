#include <stdio.h>
#include "symbolTable.h"

extern int yyparse();
extern void yyset_in(FILE *);

// Defined in grammar.y
// Returns the symbol table the parser uses
extern SCOPE** get_symbol_table();

FILE *inputFile;

int main(int argc, char* argv[])
{
    if (argc == 1) {
        printf("No program to compile\n");
        return 1;
    }
    else {
        const char* program = argv[1];
        inputFile = fopen(program, "r");
        yyset_in(inputFile);
        printf("yyparse exit code: %d\n", yyparse());
        print_symbol_table(*get_symbol_table());
        return 0;
    }

    return 0;
}