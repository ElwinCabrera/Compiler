%{
#include <stdio.h>

void yyerror(char*);
int yylex();

%}
/* Where the grammar starts */
%start program


%union {
    int id;
    int integer;
    int boolean;
    double real;
    char character;
    char* string;
}

/* Define types of tokens */
%type <character> ID; //
%type <integer> C_INTEGER; //
%type <real> C_REAL; //
%type <boolean> C_TRUE; //
%type <boolean> C_FALSE; //
%type <character> C_CHARACTER; //
%type <string> C_STRING; //

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

definition_list: /* Empty String */
    | definition definition_list
    ;

definition: /* Only permitted at top level defines a new type or function*/
    TYPE identifier COLON dblock /* record type*/
    | TYPE identifier COLON constant ARROW identifier /* arraytype. first identifier is name of arraytype. constant is the array dimensions. the last identifier is name of element type */
    | TYPE identifier COLON constant ARROW identifier COLON L_PARENTHESIS constant R_PARENTHESIS /* arraytype. first identifier is name of arraytype. constant is the array dimensions. the last identifier is name of element type. optional constant  for initialization of array */
    | TYPE identifier COLON pblock ARROW identifier /* function type*/
    | FUNCTION identifier COLON identifier sblock /* function definition. identifier1 is function name, identifier 2 is function type*/
    ;

sblock: /* sblock allows local decelerations in optional dblock*/
    L_BRACE dblock statement_list R_BRACE
    | L_BRACE statement_list R_BRACE
    ;

dblock:
    L_BRACKET declaration_list R_BRACKET
    ;

declaration_list:
    declaration SEMI_COLON declaration_list
    | declaration
    ;

declaration:
    identifier COLON identifier_list /*LHS is type, RHS is list of variable names*/
    ;

identifier_list:
    identifier assignOp constant COMMA identifier_list
    | identifier COMMA identifier_list
    | identifier assignOp constant
    | identifier
    ;

identifier:
    ID
    | type_specifier
    ;

type_specifier:
    T_BOOLEAN
    | T_CHARACTER
    | T_INTEGER
    | T_REAL
    | T_STRING
    ;

constant:
    C_INTEGER
    | C_REAL
    | C_CHARACTER
    | C_STRING
    | C_TRUE
    | C_FALSE
    ;

statement_list:
    statement statement_list
    | statement
    ;

statement:
    basic_statement
    | if_statement
    | for_statement
    | while_statement
    | switch_statement
    | sblock
    ;

basic_statement:
    assignable assignOp expression SEMI_COLON
    | memOp assignable SEMI_COLON
    ;

if_statement:
    IF L_PARENTHESIS expression R_PARENTHESIS THEN sblock ELSE sblock
    ;

for_statement:
    FOR L_PARENTHESIS statement SEMI_COLON expression SEMI_COLON statement R_PARENTHESIS sblock
    ;

while_statement:
    WHILE L_PARENTHESIS expression R_PARENTHESIS
    ;

switch_statement:
    SWITCH L_PARENTHESIS expression R_PARENTHESIS case_list OTHERWISE COLON sblock
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
    | assignable ablock
    | assignable recOp identifier
    ;

expression:
    expression_with_precedence
    ;

expression_with_precedence:
    expression_unary;

simple_expression:
    constant
    | assignable
    ;

/* Parenthesis are highest precedence */

expression_parenthetical:
    L_PARENTHESIS simple_expression R_PARENTHESIS
    | simple_expression
    ;

expression_binary:
    expression_parenthetical binaryOperator expression_parenthetical
    | expression_parenthetical
    ;

expression_unary:
    preUnaryOperator expression_binary
    | expression_binary postUnaryOperator
    | expression_binary
    ;

pblock:
    L_PARENTHESIS parameter_list R_PARENTHESIS
    ;

parameter_list:
    | non_empty_parameter_list
    ;

non_empty_parameter_list:
    parameter_declaration COMMA non_empty_parameter_list
    | parameter_declaration
    ;

parameter_declaration:
    identifier COLON identifier
    ;

ablock:
    L_PARENTHESIS argument_list R_PARENTHESIS
    ;

argument_list: /* Empty String */
    | non_empty_argument_list
    ;

non_empty_argument_list:
    expression COMMA non_empty_argument_list
    | expression
    ;

preUnaryOperator:
    SUB_OR_NEG
    | NOT
    | INT2REAL
    | REAL2INT
    ;

postUnaryOperator:
    IS_NULL
    ;

memOp:
    RESERVE
    | RELEASE
    ;

assignOp:
    ASSIGN;

recOp:
    DOT;

binaryOperator:
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
