#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <stddef.h>
#include <stdio.h>
#include "types.h"

typedef struct symtab {
   char* name;
   char* extra;
   int meta;
   struct symtype* type;
   struct scope* scope;
   struct symtab* next;
} SYMTAB;

typedef struct scope {
    int id;
    struct symtab* symbols;
    struct scope_list* children;
    struct scope* parent;
} SCOPE;

typedef struct scope_list {
    struct scope* node;
    struct scope_list* next;
} SCOPE_LIST;

SCOPE* new_scope(SCOPE*, int);
SCOPE* exit_scope(SCOPE*);

SYMTAB* last_entry(SYMTAB*);
SYMTAB* find_in_scope(SCOPE*, char*);
SYMTAB* find_in_children(SCOPE *, char*);
SYMTAB* find_entry(SYMTAB*, char*);
SYMTAB* add_symbols_to_scope(SCOPE*, SYMTAB*);
SYMTAB* add_symbols(SYMTAB* dest, SYMTAB* src);
SYMTAB* new_symbol(SYMTYPE*, char*, int, char*);
void print_symbol(SYMTAB*, int, FILE*);
void print_symbol_table(SCOPE*, FILE*);

#endif