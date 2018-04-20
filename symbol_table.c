#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "types.h"
#include "symbol_table.h"
#include "stack.h"
#include "linked_list.h"

SCOPE* new_scope(SCOPE* parent, int id)
{
  SCOPE *new = malloc(sizeof(SCOPE));

  new->id = id;
  new->children = NULL;
  new->symbols = NULL;
  new->parent = parent;

  if(parent) {
    parent->children = ll_insertfront(parent->children, new);
  }

  //printf("New scope %p is a child of %p\n", new, parent);
  return new;
}

SCOPE* exit_scope(SCOPE* current) 
{
  if(!current) {
    return NULL;
  }
  //printf("Leaving scope %p to %p\n", current, current->parent);
  return current->parent;
}

/*
    When we add to a scope, we're nesting inward. However, to check if
    something exists in a scope, we have to check the symbol table of
    every parent if it doesn't exist in the current scope.
*/
SYMTAB* find_in_scope(SCOPE* s, char* target) {

  if(s == NULL) {
    return NULL;
  }

  SYMTAB* sym = find_entry(s->symbols, target);

  if(sym == NULL) {
    sym = find_in_scope(s->parent, target);
  }

  return sym;
}

/*
  Searches down a symbol table from a parent node
*/
SYMTAB* find_in_children(SCOPE* s, char* target)
{
  if(s == NULL) {
    return NULL;
  }

  SYMTAB* sym = find_entry(s->symbols, target);

  if(sym == NULL) {
   LINKED_LIST* child = s->children;
    while(child) {
      sym = find_in_children(ll_value(child), target);
      if (sym != NULL) {
        return sym;
      }
      child = ll_next(child);
    }
  }

  return sym;
}

SYMTAB* new_symbol(SYMTYPE* type, char* name, int meta, char* extra) {
  SYMTAB *insertNew  = malloc(sizeof(SYMTAB));
  
  if(extra) { insertNew->extra = strdup(extra); } 

  insertNew->name = strdup(name);
  insertNew->meta = meta;
  insertNew->type = type;
  insertNew->scope = NULL;

  return insertNew;
}

LINKED_LIST* add_symbols_to_scope(SCOPE* scope, LINKED_LIST* symbols)
{
  if(!scope) {
    return NULL;
  }

  scope->symbols = ll_combine(scope->symbols, symbols);

  return scope->symbols;
}

bool name_match_symbol(LINKED_LIST* l, void* name) {
  SYMTAB* s = ll_value(l);
  return s && s->name && strcmp(s->name, name) == 0;
}

SYMTAB* find_entry(LINKED_LIST* start, char* name)
{
  return ll_find(start, name, name_match_symbol);
}

bool size_comparator(LINKED_LIST* a, LINKED_LIST* b) {
  SYMTAB* a_val = ll_value(a);
  SYMTAB* b_val = ll_value(b);

  return get_type_width(a_val->type) < get_type_width(b_val->type) ? false : true;
}

void reorder_symbols(SCOPE* s) {
  if(s) {
    ll_mergesort(&s->symbols, size_comparator);
  }
}

void print_symbol_table(SCOPE* symbol_table, FILE* f) {
  if(!symbol_table) {
    return;
  }

  LINKED_LIST* s = symbol_table->symbols;
  
  while(s) {
    SYMTAB* symbol = ll_value(s);
    print_symbol(symbol, symbol_table->id, f);
    s = ll_next(s);
  }
  
  LINKED_LIST* children = symbol_table->children;

  while(children) {
    SCOPE* scope = ll_value(children);
    print_symbol_table(scope, f);
    children = ll_next(children);
  }
}

void print_symbol(SYMTAB * symbol, int scope, FILE* f) {
  if(!symbol) {
    return;
  }

  if(f) {
    fprintf(f, "%s : %d : %s : %s\n", symbol->name, scope,  symbol->type ? symbol->type->name : "NULL", symbol->extra);
  } else {
    printf("%s : %d : %s : %s\n", symbol->name, scope, symbol->type ? symbol->type->name : "NULL", symbol->extra);
  }

}

