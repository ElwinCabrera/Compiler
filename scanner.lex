%{   
#include "tokenDef.h"
%}

%option yylineno

/* Anything from 0 to 9 */
DIGIT        [0-9]             
/* Has either a plus or minus sign, or not, with a sequence of digits */                                     
INTEGER      ("+"|"-")?{DIGIT}+                              
/*Basically INTEGER, followed with "." and a number of digits, followed by e|E with an interger*/       
REAL         ("+"|"-")?{DIGIT}+\.{DIGIT}+(("e"|"E"){INTEGER}+)?     
/*newline, tab, backlash, single and double quote*/
ESC          ["\n""\t""\\"""\"""\'""\r"]               
/*any alphabet character (CAPS or not), space, or escape character*/             
CHAR         \"[a-zA-Z {ESC}]\"                                       
/*An arbitrary sequence of characters length >0, not inclusing newline*/
STRING       \"[^\n {CHAR}]+\"                                        
/*An arbitrary sequence of characters, with length >=0, including whatever escape characters, enclosed with (* *) */
COMMENT      "(*"{CHAR}*"*)" 
/*Starts with a lower/upper case/"_" then continue with a lower/upper case/"_"/digit */
ID           [a-zA-Z_][a-zA-Z {DIGIT}_]*                            

    /* TODO: DEFINE THESE

    DIGIT		-Check
    INTEGER		-Check
    REAL        -Check
    CHAR        -Check
    STRING      -Check
    ID          -Check
    C_START     
    C_END       
    C_SIMPLE    
    C_COMPLEX   
    COMMENT     -Check
    ESC         -Check. These are escaped characters    

    */

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