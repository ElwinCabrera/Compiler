#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include "symbol_table.h"
#include "intermediate_code.h"
#include "code_blocks.h"
#include "linked_list.h"
#include "assembly.h"
#include "asc.h"

extern int yyparse();
extern void yyset_in(FILE *);
extern void* set_asc_file(FILE*);
extern bool check_error_status();

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
        bool blocks = false;
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
            } else if(strcmp(argv[i],"-bl") == 0) {
                blocks = true;
                //Print blocks
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
            asc_init(asc_file);
        }

        yyset_in(inputFile);
        
        if(yyparse()) {
            printf("WARNING: Unhandled syntax errors in compilation");
        }

        asc_print_file();

        if(st) { //specify to print symbol table
            char* symbol_file_path = malloc(strlen(program) + 4);
            sprintf(symbol_file_path, "%s%s", program, ".st");
            FILE* symbol_file = fopen(symbol_file_path, "w");
            if(!symbol_file) {
                printf("ERROR(%d): Could not open file %s for writing\n", errno, symbol_file_path);
            }
            free(symbol_file_path);
            print_symbol_table(get_symbol_table(), symbol_file);
            fclose(symbol_file);
        }
        
        INTERMEDIATE_CODE* code_table = get_intermediate_code();

        if(ir) { //specify to print ir
            char* ir_file_path = malloc(strlen(program) + 4);
            sprintf(ir_file_path, "%s%s", program, ".ir");
            FILE* ir_file = fopen(ir_file_path, "w");
            if(!ir_file) {
                printf("ERROR(%d): Could not open file %s for writing\n", errno, ir_file_path);
            }
            free(ir_file_path);
            print_intermediate_code(code_table, ir_file);
            fclose(ir_file);
        }

        LINKED_LIST* code_blocks = make_blocks(code_table);

        if(blocks) {
            char* bl_file_path = malloc(strlen(program) + 4);
            sprintf(bl_file_path, "%s%s", program, ".bl");
            FILE* bl_file = fopen(bl_file_path, "w");
            if(!bl_file) {
                printf("ERROR(%d): Could not open file %s for writing\n", errno, bl_file_path);
            }
            free(bl_file_path);
            print_blocks(code_blocks, bl_file);
            fclose(bl_file);
        }

        if(check_error_status() == 0) {
            process_code_blocks(code_blocks);
            char* output_file_path = malloc(strlen(program) + 3);
            sprintf(output_file_path, "%s%s", program, ".s");
            FILE* output_file = fopen(output_file_path, "w");
            if(!output_file) {
                printf("ERROR(%d): Could not open file %s for writing\n", errno, output_file_path);
            }
            free(output_file_path);
            print_asm_code(output_file);
            //print_asm_code(NULL);
            fclose(output_file);
        }

        return 0;
    }

    return 0;
}