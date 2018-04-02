#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "symbolTable.h"

extern int yyparse();
extern void yyset_in(FILE *);

// Defined in grammar.y
// Returns the symbol table the parser uses
extern SCOPE** get_symbol_table();
extern void* print_asc(const char*);

FILE *inputFile;

int main(int argc, char* argv[])
{
    if (argc == 1) {
        printf("No program to compile\n");
        return 1;
    } else {
        const char* program;
        bool asc = false;
        bool st = false;
        bool read_program = false;

        for(int i = 1;i< argc;i++) {
            if(strcmp(argv[i], "-asc") == 0) {
                asc = true;
                //DO annotated source code
            } else if(strcmp(argv[i],"-st") == 0) {
                st = true;
                //Print symbol table
            } else {
                program = argv[i];
                read_program = true;
            }
        }

        if(!read_program){
            printf("No program to compile\n");
            return 1;
        }

        inputFile = fopen(program, "r");

        if(!inputFile) {
            printf("Specified file not found\n");
            return 1;
        }

        if(asc) {
            print_asc(program);
        }

        yyset_in(inputFile);
        printf("yyparse exit code: %d\n", yyparse());

        if(st) { //specify to print symbol table
            print_symbol_table(*get_symbol_table());
        }


        return 0;
    }

    return 0;
}