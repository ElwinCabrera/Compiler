%{
#include <stdio.h>
#include "symbolTable.h"

void yyerror(char*);
int yylex();

struct Node {
  int id;
  int integer;
  int boolean;
  double real;
  char character;
  char* string;
};
    
%}

%start program


%union {
    int id;
    int integer; 
    int boolean;
    double real;
    char character;
    char* string;
    struct Node n;
}

%type <n> constant
%type <n> type_specifier
%type <string> identifier

%token <id> ID;
%token <integer> C_INTEGER;
%token <real> C_REAL;
%token <boolean> C_TRUE;
%token <boolean> C_FALSE;
%token <character> C_CHARACTER;
%token <string> C_STRING;

%left '+' '-'
%left '*' '/' '%'
%right pre_unary_prec

%token T_INTEGER
%token T_REAL
%token T_BOOLEAN
%token T_CHARACTER
%token T_STRING

%token NULL_PTR
%token RESERVE
%token RELEASE
%token FOR
%token WHILE
%token IF
%token THEN
%token ELSE
%token SWITCH
%token CASE
%token OTHERWISE
%token TYPE
%token FUNCTION
%token CLOSURE

%token L_PARENTHESIS
%token R_PARENTHESIS
%token L_BRACKET
%token R_BRACKET
%token L_BRACE
%token R_BRACE
%token S_QUOTE
%token D_QUOTE

%token SEMI_COLON
%token COLON
%token COMMA
%token ARROW
%token BACKSLASH

%token ADD
%token SUB_OR_NEG
%token MUL
%token DIV
%token REM
%token DOT
%token LESS_THAN
%token EQUAL_TO
%token ASSIGN
%token INT2REAL
%token REAL2INT
%token IS_NULL
%token NOT
%token AND
%token OR

%token COMMENT;

%%

program: 
    definition_list sblock
    ;

definition_list: 
    /* Empty String */
    | definition definition_list
    ;

definition:
    TYPE identifier COLON constant ARROW type_specifier COLON L_PARENTHESIS constant R_PARENTHESIS
    | TYPE identifier COLON constant ARROW type_specifier
    | TYPE identifier COLON pblock ARROW type_specifier
    | FUNCTION identifier COLON type_specifier sblock
    | TYPE identifier COLON dblock
    ;

pblock:
    L_PARENTHESIS parameter_list R_PARENTHESIS
    ;

parameter_list:
    /* Empty String */
    | non_empty_parameter_list
    ;

non_empty_parameter_list:
    parameter_declaration COMMA non_empty_parameter_list
    | parameter_declaration
    ;

parameter_declaration:
    type_specifier COLON identifier
    ;


dblock:
    L_BRACKET declaration_list R_BRACKET
    ;

declaration_list:
    declaration SEMI_COLON declaration_list
    | declaration
    ;

declaration:
    type_specifier COLON identifier_list
    ;


identifier_list:
    | identifier assign_op constant COMMA identifier_list
    | identifier assign_op constant
    | identifier COMMA identifier_list
    | identifier
    ;

sblock:
    L_BRACE dblock statement_list R_BRACE
    | L_BRACE statement_list R_BRACE
    ;
    
statement_list:
    statement statement_list;
    | statement;

statement:
    FOR L_PARENTHESIS statement SEMI_COLON expression SEMI_COLON statement R_PARENTHESIS sblock
    | SWITCH L_PARENTHESIS expression R_PARENTHESIS case_list OTHERWISE COLON sblock
    | IF L_PARENTHESIS expression R_PARENTHESIS THEN sblock ELSE sblock
    | WHILE L_PARENTHESIS expression R_PARENTHESIS sblock
    | assignable assign_op expression SEMI_COLON
    | mem_op assignable SEMI_COLON
    | sblock
    ;

case_list:
    case case_list
    | case
    ;

case:
    CASE constant COLON sblock
    ;

assignable:
    identifier
    | assignable rec_op identifier
    | assignable ablock
    ;

ablock:
    L_PARENTHESIS argument_list R_PARENTHESIS
    ;

argument_list:
    /* Empty String */
    | non_empty_argument_list;

non_empty_argument_list:
    expression COMMA non_empty_argument_list
    | expression
    ;

expression:
    expression binary_operator expression
    | expression post_unary_operator
    | pre_unary_operator expression %prec pre_unary_prec
    | L_PARENTHESIS expression R_PARENTHESIS
    | constant
    | assignable
    ;


identifier:
    ID
    ;

type_specifier:
    | identifier
    | T_BOOLEAN
    | T_CHARACTER
    | T_INTEGER
    | T_REAL
    | T_STRING
    ;

constant:
    C_INTEGER             {$$.integer=$1; printf("%d", $1);}
    | C_REAL              {$$.real=$1; printf("%f", $1);}
    | C_CHARACTER         {$$.character=$1; printf("%c", $1);}
    | C_STRING            {$$.string=$1; printf("%s", $1);}
    | C_TRUE              {$$.boolean=$1; printf("%d", $1);}
    | C_FALSE             {$$.boolean=$1; printf("%d", $1);}
    ;
mem_op:
    RESERVE
    | RELEASE
    ;

rec_op:
    DOT
    ;

assign_op:
    ASSIGN;

post_unary_operator:
    IS_NULL
    ;

pre_unary_operator:
    SUB_OR_NEG
    | NOT
    | INT2REAL
    | REAL2INT
    ;

binary_operator:
    ADD
    | SUB_OR_NEG
    | MUL
    | DIV
    | REM
    | AND
    | OR
    | LESS_THAN
    | EQUAL_TO
    ;

%%

void yyerror (char *s) {
   printf ("%s\n", s);
 }
