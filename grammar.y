%{

%}

%start program


%union {
    int integer; 
    int boolean;
    float real;
    char character;
    char* string;
}

%token <integer> C_INTEGER;
%token <float> C_REAL;
%token <bool> C_TRUE;
%token <bool> C_FALSE;
%token <character> C_CHARACTER;
%token <string> C_STRING;

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

%%

program:
    | definition_list sblock
    ;

definition_list:
    | definition definition_list
    ;

definition:

    ;

sblock:
    ;

dblock:
    ;

declaration_list:
    ;

declaration:
    ;

identifier_list:
    ;

identifier:
    ;

statement_list:
    ;

statement:
    ;

assignable:
    ;

expression:
    ;

pblock:
    ;

parameter_list:
    ;

non_empty_parameter_list:
    ;

parameter_declaration:
    ;

ablock:
    ;

argument_list:
    ;

non_empty_argument_list:
    ;

preUnaryOperator:
    ;

postUnaryOperator:
    ;

memOp:
    ;

assignOp:
    ;

recOp:
    ;

binaryOperator:
    ;