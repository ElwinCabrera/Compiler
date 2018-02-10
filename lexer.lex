%{   
    #include "define_tokens.h"

    int handle_token(int token)
    {
        printf("%d %s\n", token, yytext);
        return token;
    }
%}

%option yylineno

    /* TODO: DEFINE THESE

    DIGIT		
    INTEGER		
    REAL        
    CHAR        
    STRING      
    ID          
    C_START     
    C_END       
    C_SIMPLE    
    C_COMPLEX   
    COMMENT     

    */

%%

    /* Primitive Values */

{INTEGER}                   handle_token(C_INTEGER);
{REAL}                      handle_token(C_REAL);
{CHAR}                      handle_token(C_CHARACTER);
{STRING}                    handle_token(C_STRING);
"true"                      handle_token(C_TRUE);
"false"                     handle_token(C_FALSE);


    /* Comments */

{COMMENT}                   handle_token(COMMENT);

    /* Keywords */

"integer"                   handle_token(T_INTEGER);
"real"                      handle_token(T_REAL);
"character"                 handle_token(T_CHARACTER);
"string"                    handle_token(T_STRING);
"Boolean"                   handle_token(T_BOOLEAN);

"reserve"                   handle_token(RESERVE);
"release"                   handle_token(RELEASE);
"for"                       handle_token(FOR);
"while"                     handle_token(WHILE);
"if"                        handle_token(IF);
"then"                      handle_token(THEN);
"else"                      handle_token(ELSE);
"switch"                    handle_token(SWITCH);
"case"                      handle_token(CASE);
"otherwise"                 handle_token(OTHERWISE);
"type"                      handle_token(TYPE);
"function"                  handle_token(FUNCTION);
"closure"                   handle_token(CLOSURE);


    /* Punctuation */


"("                         handle_token(L_PARENTHESIS);
")"                         handle_token(R_PARENTHESIS);
"["                         handle_token(L_BRACKET);
"]"                         handle_token(R_BRACKET);
"{"                         handle_token(L_BRACE);
"}"                         handle_token(R_BRACE);
";"                         handle_token(SEMI_COLON);
":"                         handle_token(COLON);
","                         handle_token(COMMA);
"->"                        handle_token(ARROW);
"'"                         handle_token(S_QUOTE);
"\\"                        handle_token(D_QUOTE);
"\""                        handle_token(BACKSLASH);


    /* Operators */

"+"                         handle_token(ADD);
"-"                         handle_token(SUB_OR_NEG);
"*"                         handle_token(MUL);
"/"                         handle_token(DIV);
"%"                         handle_token(REM);
"."                         handle_token(DOT);
"<"                         handle_token(LESS_THAN);
"="                         handle_token(EQUAL_TO);
":="                        handle_token(ASSIGN);
"i2r"                       handle_token(INT2REAL);
"r2i"                       handle_token(REAL2INT);
"isNull"                    handle_token(IS_NULL);
"!"                         handle_token(NOT);
"&"                         handle_token(AND);
"|"                         handle_token(OR);

    /* ID */

{ID}                        handle_token(ID);

    /* Whitespace */

[ \t\r\n]                   ;

%%
