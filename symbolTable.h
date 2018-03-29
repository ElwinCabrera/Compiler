#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <stddef.h>

struct symtab {
   int id;
   char * name;
   char * type;
   char * extra;
   struct symtab * next;
};

struct scope {
    struct symtab * symbols;
    struct scope_list * children;
    struct scope * parent;
} * SYMBOL_TABLE;

struct scope_list {
    struct scope * node;
    struct scope_list * next;
};

struct scope * new_scope(struct scope *);
struct scope * exit_scope(struct scope *);

struct symtab * last_entry(struct symtab *);
struct symtab * find_in_scope(struct scope *, char*);
struct symtab * find_in_children(struct scope *, char*);
struct symtab * find_entry(struct symtab *, char*);
struct symtab * add_entry(struct scope *, char*, char*, char*);

#endif