#include <stdin.h>

extern int yylex();
extern int yylineno;
extern char* yytext;

int main(int argc, char* argv[])
{
    yyin = fopen(argv[1],"r");
    //do some parsing with yylex
    fclose(yyin);

    return 0;
}

int handle_token(int token)
{
        printf("%d %s\n", token, yytext);
        return token;
}