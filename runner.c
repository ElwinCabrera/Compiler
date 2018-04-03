#include <stdio.h>
#include "symbolTable.h"

typedef int bool;
#define true 1
#define false 0

extern int yyparse();
extern void yyset_in(FILE *);
extern SCOPE* symbols;

FILE *inputFile;

int main(int argc, char* argv[])
{
    // "symbols" global var is defined in symbolTable.h
    // It mainins the pointer to the current symbol table entry
    // for use by yacc/bison

    symbols = new_scope(NULL);
   
    if (argc == 1) {
        printf("No program to compile\n");
        return 1;
    }
    else {
        const char* program;
        bool asc = false;
        bool st = false;
        bool read_program = false;

        for(int i = 1;i< argc;i++){
            if(argv[i]=="-asc"){
                asc = true;
                //DO annotated source code
            }
            else if(argv[i]=="-st"){
                st = true;
                //Print symbol table
            }
            else{
                program = argv[i];
                read_program = true;
            }
        }
        if(!read_program){
            printf("No program to compile\n");
            return 1;
        }

        inputFile = fopen(program, "r");
        yyset_in(inputFile);


        printf("yyparse exit code: %d\n", yyparse());
        if(st){//specify to print symbol table
            print_symbol_table(symbols);
        }
        return 0;
    }

    return 0;
}