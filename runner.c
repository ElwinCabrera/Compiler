#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include "symbol_table.h"

extern int yyparse();
extern void yyset_in(FILE *);

// Defined in grammar.y
// Returns the symbol table the parser uses
extern SCOPE** get_symbol_table();
extern void* set_asc_file(FILE*);

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
        bool ir = false;
        bool read_program = false;

        for(int i = 1;i< argc;i++) {
            if(strcmp(argv[i], "-asc") == 0) {
                asc = true;
                //DO annotated source code
            } else if(strcmp(argv[i],"-st") == 0) {
                st = true;
                //Print symbol table
            } else if(strcmp(argv[i],"-ir") == 0) {
                ir = true;
                //Print intermediate representation
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
            char* asc_file_path = malloc(strlen(program) + 5);
            sprintf(asc_file_path, "%s%s", program, ".asc");
            FILE* asc_file = fopen(asc_file_path, "w");
            if(!asc_file) {
                printf("ERROR(%d): Could not open file %s for writing\n", errno, asc_file_path);
            }
            free(asc_file_path);
            set_asc_file(asc_file);
        }

        yyset_in(inputFile);
        printf("yyparse exit code: %d\n", yyparse());

        if(st) { //specify to print symbol table
            char* symbol_file_path = malloc(strlen(program) + 4);
            sprintf(symbol_file_path, "%s%s", program, ".st");
            FILE* symbol_file = fopen(symbol_file_path, "w");
            if(!symbol_file) {
                printf("ERROR(%d): Could not open file %s for writing\n", errno, symbol_file_path);
            }
            free(symbol_file_path);
            print_symbol_table(*get_symbol_table(), symbol_file);
        }

        if(ir) { //specify to print symbol table
            char* ir_file_path = malloc(strlen(program) + 4);
            sprintf(ir_file_path, "%s%s", program, ".ir");
            FILE* ir_file = fopen(ir_file_path, "w");
            if(!ir_file) {
                printf("ERROR(%d): Could not open file %s for writing\n", errno, ir_file_path);
            }
            free(ir_file_path);
            print_intermediate_code(get_intermediate_code(), ir_file);
        }


        return 0;
    }

    return 0;
}