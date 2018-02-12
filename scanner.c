#include <stdio.h>

extern int yyleng;
extern int yylineno;
extern char* yytext;

extern int yylex();
extern void yyset_in(FILE*);


static int row = 1;
static int column = 1;
static int end_row = 1;
static int end_column = 1;

void update_location(void);
int handle_token(int token);

int main(int argc, char* argv[])
{
    if(argc < 1)
    {
        printf("Specify an input file.\n");
        return 1;
    }

    FILE *input_file = fopen(argv[1],"r");

    if(!input_file)
    {
        printf("The input file was not found.\n");
        return 1;
    }

    yyset_in(input_file);
    
    int token;
    while(token = yylex())
    {
        update_location();
        handle_token(token);
    }

    fclose(input_file);

    return 0;
}



int handle_token(int token)
{
    if(token > 0)
    {
        printf("%d %s %d %d\n", token, yytext, row, column);
    }

    return token;
}

void update_location(void)
{
    row = end_row;           
    column = end_column;     
                    
    if(end_row != yylineno)  
    {                        
        end_column = 1;      
        end_row = yylineno;  
    }                        
    else                     
    {                        
        end_column += yyleng;
    }                        
}