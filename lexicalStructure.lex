%{
#include "y.tab.h"
#include "stack.h"
#include "ir.h"
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
{CHAR}                      yylval.character = yytext[1]; return handle_token(C_CHARACTER);
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

"reserve"                   yylval.integer = I_RESERVE; return handle_token(RESERVE);
"release"                   yylval.integer = I_RELEASE; return handle_token(RELEASE);
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

"+"                         yylval.integer = I_ADD; return handle_token(ADD);
"-"                         yylval.integer = I_SUB; return handle_token(SUB_OR_NEG);
"*"                         yylval.integer = I_MULTIPLY; return handle_token(MUL);
"/"                         yylval.integer = I_DIVIDE; return handle_token(DIV);
"%"                         yylval.integer = I_MODULUS; return handle_token(REM);
"."                         yylval.integer = DOT; return handle_token(DOT);
"<"                         yylval.integer = I_LESS_THAN; return handle_token(LESS_THAN);
"="                         yylval.integer = I_EQUAL; return handle_token(EQUAL_TO);
":="                        yylval.integer = I_ASSIGN; return handle_token(ASSIGN);
"i2r"                       yylval.integer = I_INT2REAL; return handle_token(INT2REAL);
"r2i"                       yylval.integer = I_REAL2INT; return handle_token(REAL2INT);
"isNull"                    yylval.integer = I_IS_NULL; return handle_token(IS_NULL);
"!"                         yylval.integer = I_NOT; return handle_token(NOT);
"&"                         yylval.integer = I_AND; return handle_token(AND);
"|"                         yylval.integer = I_OR; return handle_token(OR);

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

extern STACK** get_errors();

void set_asc_file(FILE* f) {
    asc_file = f;
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
        STACK** err = get_errors();

        while(*err) {
            char* e = stack_peek(*err);
            if(asc_file) {
                fprintf(asc_file, "%s", e);
            }
            printf("%s", e);
            free(e);
            *err = stack_pop(*err);
        }
    }

    return token;
}
