#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "symbolTable.h"

struct scope * new_scope(struct scope * parent)
{
  struct scope * new = malloc(sizeof(struct scope));

  new->parent = parent;

  return new;
}

struct scope * exit_scope(struct scope * current) 
{
  if(!current) {
    return NULL;
  }

  return current->parent;
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

void add_entry(struct scope *start, int type, char* name, char *extra)
{
   struct symtab *insertNew  = malloc(sizeof(struct symtab));;

   insertNew->id = 1;
   insertNew->extra = extra;
   insertNew->name = name;
   insertNew->type = type;
   insertNew->next = start->symbols;

   start->symbols = insertNew;
   
}

struct symtab *find_entry(struct symtab *start, char* name)
{
  //if the first node will never have any data then I need to skip the first node
   struct symtab *p;
   p = start;
   while(p  != NULL) {
      if(strcmp(p->name, name) == 0) {
         return p;
      }
      p = p->next;
   }

  return NULL;
}
