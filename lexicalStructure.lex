%{
#include "y.tab.h"
#include "errors.h"
int handle_token(int);
%}

%option yylineno
%option noyywrap
%option noinput
%option nounput

/* Anything from 0 to 9 */
DIGIT                       [0-9]             
/* Has either a plus or minus sign, or not, with a sequence of digits */                                     
INTEGER                     {DIGIT}+                              
/*Basically INTEGER, followed with "." and a number of digits, followed by e|E with an interger*/       
REAL                        {INTEGER}+"."{DIGIT}+(("e"|"E")" "?{INTEGER}+)?           
/*single ascii character enclosed in single quotations, may or may not be backslashed*/             
CHAR                        "\'"\\?(.|" ")"\'"                                       
/*An arbitrary sequence of characters length >0, not inclusing newline*/

STRING                      \"([^\"\n]|"\\\"")*"\""



COMMENT_START               "(*"
COMMENT_END                 "*)"
ANYTHING_BUT_STAR           [^*]
STAR_IF_NO_RPARENTHESIS     "*"[^\)]
/* Comments start with (* and end with *) and captures everything that isn't a * followed by ) */
COMMENT                     {COMMENT_START}({ANYTHING_BUT_STAR}|{STAR_IF_NO_RPARENTHESIS})*?{COMMENT_END}
/*Starts with a lower/upper case/"_" then continue with a lower/upper case/"_"/digit */
ID                          [a-zA-Z_][a-zA-Z0-9_]*                            

%%

    /* Primitive Values */

{INTEGER}                   yylval.integer = atoi(yytext); return handle_token(C_INTEGER);
{REAL}                      yylval.real = atof(yytext); return handle_token(C_REAL);
{CHAR}                      yylval.character = yytext[0]; return handle_token(C_CHARACTER);
{STRING}                    yylval.string = strdup(yytext); return handle_token(C_STRING);
"true"                      yylval.boolean = 1; return handle_token(C_TRUE);
"false"                     yylval.boolean = 0; return handle_token(C_FALSE);


    /* Comments */

{COMMENT}                   handle_token(COMMENT);

    /* Keywords */

"integer"                   yylval.string = strdup(yytext); return handle_token(T_INTEGER);
"real"                      yylval.string = strdup(yytext); return handle_token(T_REAL);
"character"                 yylval.string = strdup(yytext); return handle_token(T_CHARACTER);
"string"                    yylval.string = strdup(yytext); return handle_token(T_STRING);
"Boolean"                   yylval.string = strdup(yytext); return handle_token(T_BOOLEAN);

"reserve"                   return handle_token(RESERVE);
"release"                   return handle_token(RELEASE);
"for"                       return handle_token(FOR);
"while"                     return handle_token(WHILE);
"if"                        return handle_token(IF);
"then"                      return handle_token(THEN);
"else"                      return handle_token(ELSE);
"switch"                    return handle_token(SWITCH);
"case"                      return handle_token(CASE);
"otherwise"                 return handle_token(OTHERWISE);
"type"                      return handle_token(TYPE);
"function"                  return handle_token(FUNCTION);
"closure"                   return handle_token(CLOSURE);
"null"                      return handle_token(NULL_PTR);


    /* Punctuation */


"("                         return handle_token(L_PARENTHESIS);
")"                         return handle_token(R_PARENTHESIS);
"["                         return handle_token(L_BRACKET);
"]"                         return handle_token(R_BRACKET);
"{"                         return handle_token(L_BRACE);
"}"                         return handle_token(R_BRACE);
";"                         return handle_token(SEMI_COLON);
":"                         return handle_token(COLON);
","                         return handle_token(COMMA);
"->"                        return handle_token(ARROW);
"'"                         return handle_token(S_QUOTE);
"\""                        return handle_token(D_QUOTE);
"\\"                        return handle_token(BACKSLASH);


    /* Operators */

"+"                         yylval.string = strdup(yytext); return handle_token(ADD);
"-"                         yylval.string = strdup(yytext); return handle_token(SUB_OR_NEG);
"*"                         yylval.string = strdup(yytext); return handle_token(MUL);
"/"                         yylval.string = strdup(yytext); return handle_token(DIV);
"%"                         yylval.string = strdup(yytext); return handle_token(REM);
"."                         yylval.string = strdup(yytext); return handle_token(DOT);
"<"                         yylval.string = strdup(yytext); return handle_token(LESS_THAN);
"="                         yylval.string = strdup(yytext); return handle_token(EQUAL_TO);
":="                        yylval.string = strdup(yytext); return handle_token(ASSIGN);
"i2r"                       yylval.string = strdup(yytext); return handle_token(INT2REAL);
"r2i"                       yylval.string = strdup(yytext); return handle_token(REAL2INT);
"isNull"                    yylval.string = strdup(yytext); return handle_token(IS_NULL);
"!"                         yylval.string = strdup(yytext); return handle_token(NOT);
"&"                         yylval.string = strdup(yytext); return handle_token(AND);
"|"                         yylval.string = strdup(yytext); return handle_token(OR);

    /* ID */

{ID}                        yylval.string = strdup(yytext); return handle_token(ID);

    /* Whitespace and anything else*/

[ \t\r\n]                   handle_token(-1);
.                           printf("Unexpected character");

%%


static int row = 1;
static int column = 1;
static int end_row = 1;
static int end_column = 1;
static FILE* asc_file = 0;

int get_row() { return row; }
int get_column() { return column; }

extern ERROR** get_errors();

void print_asc(const char* c) {
    char* new = malloc(strlen(c) + 5);
    strcpy(new, c);
    strcat(new, ".asc");
    asc_file = fopen(new, "w");
    if(!asc_file) {
        printf("(ERROR CODE: %d) Unable to open %s\n", errno, new);
    }
    free(new);
}

/*
    Helper function for updating the file position as lex processes
    the character stream.
*/
int update_location()
{
    row = end_row;           
    column = end_column;     
                    
    if(end_row != yylineno)  
    {                        
        end_column = 1;      
        end_row = yylineno;  
        return 1;
    }                        
    else                     
    {                        
        end_column += yyleng;
        return 0;
    }                        
}

/*
    A generic handler for tokens. Takes a token, does some processing, and
    then returns it for use by lex/yacc.
*/
int handle_token(int token)
{
    if(asc_file) {
        fprintf(asc_file, "%s", yytext);
    }

    // Returns 1 on new line
    if(update_location()) {
        ERROR** err = get_errors();

        while(*err) {
            ERROR* temp = *err;
            if(asc_file) {
                fprintf(asc_file, "%s", temp->e);
            }

            printf("%s", temp->e);

            *err = pop_error(temp);
            free_error(temp);
        }
    }

    return token;
}
