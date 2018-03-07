#include <stdio.h>

// Defined by yacc, this function handles the calls to yylex
extern int yyparse();

int main(int argc, char* argv[])
{    
    printf("yyparse exit code: %d\n", yyparse());
    return 0;
}
