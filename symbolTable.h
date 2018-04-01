#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <stddef.h>
#include "types.h"

typedef struct symtab {
   int id;
   char *name;
   struct type *type;
   char *extra;
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

SCOPE *new_scope(SCOPE *);
SCOPE *exit_scope(SCOPE *);

SYMTAB *last_entry(SYMTAB *);
SYMTAB *find_in_scope(SCOPE *, char*);
SYMTAB *find_in_children(SCOPE *, char*);
SYMTAB *find_entry(SYMTAB *, char*);
SYMTAB *add_entry(SCOPE *, struct type *, char*, char*);
void print_symbol(SYMTAB *);
void print_symbol_table(SCOPE *);
void print_symbol_table_impl(SCOPE *);

#endif