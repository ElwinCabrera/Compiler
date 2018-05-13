#ifndef ASC_H
#define ASC_H

#include <stdio.h>
#include "linked_list.h"
#include "symbol_table.h"


typedef struct line {
    int lineno;
    struct scope* scope;
    char* text;
} LINE;

typedef struct asc {
    int max_prefix;
    int capacity;
    int index;
    FILE* file;
    SCOPE* scope;
    LINKED_LIST* lines;
} ASC;

ASC* get_asc();
void asc_init(FILE* f);
void asc_append(char*);
void asc_error(char*);
void asc_new_line(int);
void asc_print_file();
void asc_line_scope(struct scope*);

#endif