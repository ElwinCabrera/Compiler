%{
       

#define ID 101

#define T_INTEGER 201
#define T_REAL 202
#define T_BOOLEAN 203
#define T_CHARACTER 204
#define T_STRING 205

#define C_INTEGER 301
#define C_REAL 302
#define C_CHARACTER 303
#define C_STRING 304
#define C_TRUE 305
#define C_FALSE 306

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

#define L_PARENTHESIS 501
#define R_PARENTHESIS 502
#define L_BRACKET 503
#define R_BRACKET 504
#define L_BRACE 505
#define R_BRACE 506
#define S_QUOTE 507
#define D_QUOTE 508

#define SEMI_COLON 551
#define COLON 552
#define COMMA 553
#define ARROW 554
#define BACKSLASH 555

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

#define COMMENT 700

%}

%option yylineno

/* Anything from 0 to 9 */
DIGIT                       [0-9]             
/* Has either a plus or minus sign, or not, with a sequence of digits */                                     
INTEGER                     ("+"|"-")?{DIGIT}+                              
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

{INTEGER}                   return C_INTEGER;
{REAL}                      return C_REAL;
{CHAR}                      return C_CHARACTER;
{STRING}                    return C_STRING;
"true"                      return C_TRUE;
"false"                     return C_FALSE;


    /* Comments */

{COMMENT}                   return COMMENT;

    /* Keywords */

"integer"                   return T_INTEGER;
"real"                      return T_REAL;
"character"                 return T_CHARACTER;
"string"                    return T_STRING;
"Boolean"                   return T_BOOLEAN;

"reserve"                   return RESERVE;
"release"                   return RELEASE;
"for"                       return FOR;
"while"                     return WHILE;
"if"                        return IF;
"then"                      return THEN;
"else"                      return ELSE;
"switch"                    return SWITCH;
"case"                      return CASE;
"otherwise"                 return OTHERWISE;
"type"                      return TYPE;
"function"                  return FUNCTION;
"closure"                   return CLOSURE;
"null"                      return NULL_PTR;


    /* Punctuation */


"("                         return L_PARENTHESIS;
")"                         return R_PARENTHESIS;
"["                         return L_BRACKET;
"]"                         return R_BRACKET;
"{"                         return L_BRACE;
"}"                         return R_BRACE;
";"                         return SEMI_COLON;
":"                         return COLON;
","                         return COMMA;
"->"                        return ARROW;
"'"                         return S_QUOTE;
"\""                        return D_QUOTE;
"\\"                        return BACKSLASH;


    /* Operators */

"+"                         return ADD;
"-"                         return SUB_OR_NEG;
"*"                         return MUL;
"/"                         return DIV;
"%"                         return REM;
"."                         return DOT;
"<"                         return LESS_THAN;
"="                         return EQUAL_TO;
":="                        return ASSIGN;
"i2r"                       return INT2REAL;
"r2i"                       return REAL2INT;
"isNull"                    return IS_NULL;
"!"                         return NOT;
"&"                         return AND;
"|"                         return OR;

    /* ID */

{ID}                        return ID;

    /* Whitespace and anything else*/

[ \t\r\n]                   return -1;
.                           printf("Unexpected character");

%%

int yywrap(void){
    return 1;
}