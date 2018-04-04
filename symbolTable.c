#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "types.h"
#include "symbolTable.h"

SCOPE* new_scope(SCOPE* parent)
{
  SCOPE *new = malloc(sizeof(SCOPE));

  new->children = NULL;
  new->symbols = NULL;
  new->parent = parent;

  if(parent != NULL) {
    SCOPE_LIST *child = malloc(sizeof(SCOPE_LIST));
    child->node = new;
    child->next = parent->children;
    parent->children = child;
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
    SCOPE_LIST *child = s->children;
    while(child != NULL) {
      sym = find_in_children(child->node, target);
      if (sym != NULL) {
        return sym;
      }
      child = child->next;
    }
  }

  return sym;
}

SYMTAB* last_entry(SYMTAB* start)
{
  if(!start) {
    return NULL;
  }

  SYMTAB* p = start;
  while(p->next != NULL) {
    p = p->next;
  }
  return p;
}

SYMTAB* add_symbols(SYMTAB* dest, SYMTAB* src) {
  SYMTAB* last = last_entry(dest);
  if(last) {
    // dest has valid contents
    last->next = src;
    return dest;
  }
  // nothing found in dest
  return src;
}

SYMTAB* new_symbol(SYMTYPE* type, char* name, int meta, char* extra) {
  SYMTAB *insertNew  = malloc(sizeof(SYMTAB));
  
  if(extra) { insertNew->extra = strdup(extra); } 

  insertNew->name = strdup(name);
  insertNew->meta = meta;
  insertNew->type = type;
  insertNew->next = NULL;

  return insertNew;
}

SYMTAB* add_symbols_to_scope(SCOPE* scope, SYMTAB* symbols)
{
  if(!scope) {
    return NULL;
  }

  scope->symbols = add_symbols(scope->symbols, symbols);
  
  return scope->symbols;
}

SYMTAB* find_entry(SYMTAB* start, char* name)
{
  //if the first node will never have any data then I need to skip the first node
  SYMTAB *p = start;
  while(p  != NULL) {
    if(strcmp(p->name, name) == 0) {
      return p; 
    }
    p = p->next;
  }

  return NULL;
}

void print_symbol_table(SCOPE* symbol_table, FILE* f) {
  if(!symbol_table) {
    return;
  }

  SYMTAB* s = symbol_table->symbols;
  
  while(s) {
    print_symbol(s, f);
    s = s->next;
  }
  
  SCOPE_LIST * children = symbol_table->children;

  while(children) {
    print_symbol_table(children->node, f);
    children = children->next;
  }
}

void print_symbol(SYMTAB * symbol, FILE* f) {
  if(!symbol) {
    return;
  }

  if(f) {
    fprintf(f, "%s : %p : %p : %s\n", symbol->name, symbol->scope, symbol->type, symbol->extra);
  } else {
    printf("%s : %p : %p : %s\n", symbol->name, symbol->scope, symbol->type, symbol->extra);
  }

}
