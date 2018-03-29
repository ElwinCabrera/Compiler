#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct symtab {
   int id;
   int scope;
   char *name;
   int type;
   char *extra
   struct symtab *next;
};

enum types {
   KEYWORD = 1,
   T_INTEGER,
   T_REAL,
   T_REAL,
   T_BOOLEAN,
   T_STRING
};

struct symtab *last_entry(struct symtab *start)
{
   struct symtab *p;
   p = start;
   while(p->next != NULL) {
      p = p->next;
   }
   return p;
}

void add_entry(char* name, int type, int scope, char *extra, struct symtab *start)
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
   insertNew->scope= scope
   insertNew->extra=extra;
   insertNew->name = name;
   insertNew->type = type;
   insertNew->next = NULL;
}

struct symtab *find_entry(char* name, struct symtab *start)
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

}
