#include <stdio.h>
#include "symbolTable.h"

extern int yyparse();
extern void yyset_in(FILE *);

FILE *inputFile;
int main(int argc, char* argv[])
{
   //printf("yyparse exit code: %d\n", yyparse());
   SYMTAB *table = malloc(sizeof(SYMTAB));
   
 if (argc == 1){
     printf("No program to compile\n");
     return 1;
 }
 else{
     const char* program = argv[1];
     inputFile = fopen(program, "r");
     yyset_in(inputFile);
     yyparse();
     return 0;
 }
   return 0;
}