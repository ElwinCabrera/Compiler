#include <stdio.h>

//The followuing are defined in a diferent module, why the extern keyword is used.
extern int yyleng; /* Will give us the length to a token value */
extern int yylineno; /* Gives us more information (the line number) when we're parsing to generate a helpful error message later.*/  
extern char* yytext; /* Gives us the actual value of the tokens we're parsing*/
extern int yylex(); /* Will help us parse our file into tokens (retuns 0 if done)*/
extern void yyset_in(FILE*);

void update_location(void); /* keeps track of the current row and coulumn we're at in our file*/
int handle_token(int token); /* Used to output tokens as we are parsing them in a specific format (format is from project description)*/

static int row = 1;
static int column = 1;
static int end_row = 1;
static int end_column = 1;

int main(int argc, char* argv[])
{
    // if(argc < 1)
    // {
    //     printf("Specify an input file.");
    //     return 1;
    // }

    // FILE *input_file = fopen(argv[1],"r");

    // if(!input_file)
    // {
    //     printf("The input file was not found.");
    //     return 1;
    // }

    //yyset_in(input_file);
    
    int token;
    while(token = yylex())
    {
        update_location();
        handle_token(token);
    }

    // fclose(input_file);

    return 0;
}

void update_location()
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

int handle_token(int token)
{
    if(token > 0)
    {
        printf("%d %s %d %d\n", token, yytext, row, column);
    }

    return token;
}
