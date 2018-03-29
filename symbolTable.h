#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <stddef.h>

typedef struct symtab {
   int id;
   char *name;
   int type;
   char *extra;
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

enum types {
   KEYWORD = 1,
   T_INTEGER,
   T_REAL,
   T_BOOLEAN,
   T_CHARACTER,
   T_STRING
} TYPE;

SCOPE *new_scope(SCOPE *);
SCOPE *exit_scope(SCOPE *);

SYMTAB *last_entry(SYMTAB *);
SYMTAB *find_in_scope(SCOPE *, char*);
SYMTAB *find_in_children(SCOPE *, char*);
SYMTAB *find_entry(SYMTAB *, char*);
SYMTAB *add_entry(SCOPE *, int, char*, char*);

#endif