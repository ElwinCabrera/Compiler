#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>

typedef struct symtab {
    bool live;
    int next_use;
    char* name;
    char* extra;
    int meta;
    int width;
    struct address* label;
    struct symtype* type;
    struct scope* scope;
} SYMTAB;

typedef struct scope {
    int id;
    struct linked_list* symbols;
    struct linked_list* children;
    struct scope* parent;
} SCOPE;


SCOPE* new_scope(SCOPE*, int);
SCOPE* exit_scope(SCOPE*);

SYMTAB* find_in_scope(SCOPE*, char*);
SYMTAB* find_in_children(SCOPE *, char*);
SYMTAB* find_entry(struct linked_list*, char*);
struct linked_list* add_symbols_to_scope(SCOPE*, struct linked_list*);
SYMTAB* new_symbol(struct symtype*, char*, int, char*);
void reorder_symbols(SCOPE*);
void print_symbol(SYMTAB*, int, FILE*);
void print_symbol_table(SCOPE*, FILE*);

#endif
