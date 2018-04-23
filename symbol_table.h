#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>

typedef enum s_type {
    ST_LOCAL,
    ST_PARAMETER,
    ST_TYPE,
    ST_FUNCTION,
    ST_RETURN,
    ST_TEMPORARY,
} STYPE;

typedef struct symbol_table {
    struct scope* current_scope;
} SYMBOL_TABLE;

typedef struct symbol {
    bool live;
    int next_use;
    char* name;
    char* extra;
    STYPE meta;
    int width;
    struct address* label;
    struct symtype* type;
    struct scope* scope;
} SYMBOL;

typedef struct scope {
    int id;
    struct linked_list* symbols;
    struct linked_list* children;
    struct scope* parent;
} SCOPE;

SYMBOL_TABLE* get_symbol_table();
SCOPE* new_scope(SCOPE*, int);
SCOPE* exit_scope(SCOPE*);
SYMBOL* find_in_scope(SCOPE*, char*);
SYMBOL* find_in_children(SCOPE *, char*);
SYMBOL* find_entry(struct linked_list*, char*);
struct linked_list* add_symbols_to_scope(SCOPE*, struct linked_list*);
SYMBOL* new_symbol(struct symtype*, char*, int, char*);
void reorder_symbols(SCOPE*);
void print_symbol(SYMBOL*, int, FILE*);
void print_scope(SCOPE*, FILE*);
void print_symbol_table(SYMBOL_TABLE*, FILE*);

#endif
