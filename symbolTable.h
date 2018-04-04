#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <stddef.h>

typedef struct symtab {
   char *name;
   char *extra;
   int meta;
   struct type *type;
   struct scope *scope;
   struct symtab *next;
} SYMTAB;

typedef struct scope {
    struct symtab * symbols;
    struct scope_list * children;
    struct scope * parent;
} SCOPE;

typedef struct scope_list {
    struct scope * node;
    struct scope_list * next;
} SCOPE_LIST;

SCOPE* new_scope(SCOPE *);
SCOPE* exit_scope(SCOPE *);

SYMTAB* last_entry(SYMTAB*);
SYMTAB* find_in_scope(SCOPE*, char*);
SYMTAB* find_in_children(SCOPE *, char*);
SYMTAB* find_entry(SYMTAB*, char*);
SYMTAB* add_symbols_to_scope(SCOPE*, SYMTAB*);
SYMTAB* add_symbols(SYMTAB* dest, SYMTAB* src);
SYMTAB* new_symbol(struct type *, char*, int, char*);
void print_symbol(SYMTAB*, FILE*);
void print_symbol_table(SCOPE *, FILE*);

#endif
