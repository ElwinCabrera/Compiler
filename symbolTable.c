#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "symbolTable.h"

struct symtab *last_entry(struct symtab *start)
{
   struct symtab *p;
   p = start;
   while(p->next != NULL) {
      p = p->next;
   }
   return p;
}

struct symtab * add_entry(struct symtab *start, int type, char* name, char *extra)
{
   struct symtab *insertNew;
   insertNew = last_entry(start);
   int id=0;
   if(insertNew == start) { //do i need this since the first node will not have any symbol table data?
      insertNew = start;
   }
   else {
      insertNew = malloc(sizeof(struct symtab));
      id = last_entry(start)->id;
      last_entry(start)->next = insertNew;
   }
   insertNew->id = id + 1;
   insertNew->extra = extra;
   insertNew->name = name;
   insertNew->type = type;
   insertNew->next = NULL;

   return insertNew;
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
