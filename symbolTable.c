#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "symbolTable.h"

SCOPE *new_scope(SCOPE *parent)
{
  SCOPE *new = malloc(sizeof(SCOPE));

  new->children = NULL;
  new->parent = parent;
  
  if(parent != NULL) {
    SCOPE_LIST *child = malloc(sizeof(SCOPE_LIST));
    child->node = new;
    child->next = parent->children;
    parent->children = child;
  }

  return new;
}

SCOPE *exit_scope(SCOPE *current) 
{
  if(!current) {
    return NULL;
  }

  return current->parent;
}

/*
    When we add to a scope, we're nesting inward. However, to check if
    something exists in a scope, we have to check the symbol table of
    every parent if it doesn't exist in the current scope.
*/
SYMTAB *find_in_scope(SCOPE *s, char* target) {

  if(s == NULL) {
    return NULL;
  }

  SYMTAB *sym = find_entry(s->symbols, target);

  if(sym == NULL) {
    sym = find_in_scope(s->parent, target);
  }

  return sym;
}

/*
  Searches down a symbol table from a parent node
*/
SYMTAB *find_in_children(SCOPE *s, char* target)
{
  if(s == NULL) {
    return NULL;
  }

  SYMTAB *sym = find_entry(s->symbols, target);

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

SYMTAB *last_entry(SYMTAB *start)
{
  SYMTAB *p;
  p = start;
  while(p->next != NULL) {
    p = p->next;
  }
  return p;
}

SYMTAB *add_entry(SCOPE *start, char* type, char* name, char *extra)
{
  SYMTAB *insertNew  = malloc(sizeof(SYMTAB));

  insertNew->id = 1;

  if(type) { insertNew->type = strdup(type); }
  if(extra) { insertNew->extra = strdup(extra); }
  
  insertNew->name = strdup(name);
  insertNew->next = start->symbols;

  start->symbols = insertNew;
  return insertNew;
}

SYMTAB *find_entry(SYMTAB *start, char* name)
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
