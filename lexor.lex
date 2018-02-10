%{
    // identifier
    #define ID 101
    // type names
    #define T_INTEGER 201
    #define T_REAL 202
    #define T_BOOLEAN 203
    #define T_CHARACTER 204
    #define T_STRING 205
    // constants
    #define C_INTEGER 301
    #define C_REAL 302
    #define C_CHARACTER 303
    #define C_STRING 304
    #define C_TRUE 305
    #define C_FALSE 306
    // keywords
    #define NULL_PTR 401
    #define RESERVE 402
    #define RELEASE 403
    #define FOR 404
    #define WHILE 405
    #define IF 406
    #define THEN 407
    #define ELSE 408
    #define SWITCH 409
    #define CASE 410
    #define OTHERWISE 411
    #define TYPE 412
    #define FUNCTION 413
    #define CLOSURE 414

    // punctuation - grouping
    #define L_PARENTHESIS 501
    #define R_PARENTHESIS 502
    #define L_BRACKET 503
    #define R_BRACKET 504
    #define L_BRACE 505
    #define R_BRACE 506
    #define S_QUOTE 507
    #define D_QUOTE 508
    // punctuation - other
    #define SEMI_COLON 551
    #define COLON 552
    #define COMMA 553
    #define ARROW 554
    #define BACKSLASH 555
    // operators
    #define ADD 601
    #define SUB_OR_NEG 602
    #define MUL 603
    #define DIV 604
    #define REM 605
    #define DOT 606
    #define LESS_THAN 607
    #define EQUAL_TO 608
    #define ASSIGN 609
    #define INT2REAL 610
    #define REAL2INT 611
    #define IS_NULL 612
    #define NOT 613
    #define AND 614
    #define OR 615
    // comments
    #define COMMENT 700

    
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
"{"                         handle_token(R_BRACE);
"}"                         handle_token(L_BRACE);
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
