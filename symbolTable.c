#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "symbolTable.h"

struct scope * new_scope(struct scope * parent)
{
  struct scope * new = malloc(sizeof(struct scope));

  new->children = NULL;
  new->parent = parent;
  
  if(parent != NULL) {
    struct scope_list * child = malloc(sizeof(struct scope_list));
    child->node = new;
    child->next = parent->children;
    parent->children = child;
  }

  return new;
}

struct scope * exit_scope(struct scope * current) 
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
struct symtab * find_in_scope(struct scope * s, char* target) {

  if(s == NULL) {
    return NULL;
  }

  struct symtab * sym = find_entry(s->symbols, target);

  if(sym == NULL && s->parent != NULL) {
    sym = find_in_scope(s->parent, target);
  }

  return sym;
}

/*
  Searches down a symbol table from a parent node
*/
struct symtab * find_in_children(struct scope * s, char* target)
{

  if(s == NULL) {
    return NULL;
  }

  struct symtab * sym = find_entry(s->symbols, target);

  if(sym == NULL) {
    struct scope_list * child = s->children;
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

struct symtab *last_entry(struct symtab *start)
{
  struct symtab *p;
  p = start;
  while(p->next != NULL) {
    p = p->next;
  }
  return p;
}

struct symtab * add_entry(struct scope *start, int type, char* name, char *extra)
{
  struct symtab *insertNew  = malloc(sizeof(struct symtab));

  insertNew->id = 1;
  insertNew->type = type;
  insertNew->extra = extra;
  insertNew->name = name;
  insertNew->next = start->symbols;

  start->symbols = insertNew;
  return insertNew;
}

struct symtab *find_entry(struct symtab *start, char* name)
{
  //if the first node will never have any data then I need to skip the first node
  struct symtab *p = start;
  while(p  != NULL) {
    if(strcmp(p->name, name) == 0) {
      return p; 
    }
    p = p->next;
  }

  return NULL;
}
