%{
#include <stdio.h>
#include <string.h>
#include "symbolTable.h"
#include "types.h"
#include "errors.h"

extern char* yytext;
void yyerror(char*);
int yylex();

void type_not_found_error(char*);
void initialize_types();
SCOPE* open_scope();
SCOPE* close_scope();
void push_context(void*);
void* pop_context();
void* peek_context();
SYMTYPE* try_add_type(int, char*);
SYMTYPE* try_find_type(char*);
int try_add_symbol(SYMTYPE*, char*, char*);

%}

%start program

%union {
    int integer; 
    int boolean;
    double real;
    char character;
    char* string;
    struct scope* scope;
}

%token <string> ID;
%token <integer> C_INTEGER;
%token <real> C_REAL;
%token <boolean> C_TRUE;
%token <boolean> C_FALSE;
%token <character> C_CHARACTER;
%token <string> C_STRING;

%type <string> identifier;
%type <string> type_specifier;
%type <scope> open_scope close_scope;

// Operator precedence conflicts, but the generated state machine
// chooses the correct state, we just need to handle precedence
%expect 20

%left '+' '-'
%left '*' '/' '%'
%right pre_unary_prec

%token <string> T_INTEGER
%token <string> T_REAL
%token <string> T_BOOLEAN
%token <string> T_CHARACTER
%token <string> T_STRING

%token ARRAY
%token RECORD

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

open_scope: { $$ = open_scope(); } ;
close_scope: { $$ = close_scope(); } ;

/* 
    We may need to know certain types exist even if they haven't been defined
    yet. It may be useful to have a separate "type lookup" structure for fast
    checking to see if they exist
*/

program: 
    open_scope {
        initialize_types();
    } definition_list sblock
    ;

definition_list: 
    /* Empty String */
    | definition definition_list
    ;

definition:
    TYPE identifier COLON constant ARROW type_specifier COLON L_PARENTHESIS constant R_PARENTHESIS {
        SYMTYPE* type = try_add_type(ARRAY, $2);
        try_add_symbol(type, $2, "type");
    }
    | TYPE identifier COLON constant ARROW type_specifier {
        SYMTYPE* type = try_add_type(ARRAY, $2);
        try_add_symbol(type, $2, "type");
    }
    | TYPE identifier COLON open_scope pblock close_scope ARROW type_specifier {
        SYMTYPE* type = try_add_type(FUNCTION, $2);
        type->details.function->parameters = $4;
        type->details.function->return_type = try_find_type($8);
        try_add_symbol(type, $2, "type");
    }
    | FUNCTION identifier COLON type_specifier sblock {
        SYMTYPE* type = try_find_type($4);
        if(!type) {
            yyerrok;
        } else {
            try_add_symbol(type, $2, "function");
        }
    }
    | TYPE identifier COLON open_scope {
        try_add_type(RECORD, $2);
    } dblock close_scope  {
        SYMTYPE* type = try_find_type($2);
        type->details.record->members = $4;
        try_add_symbol(type, $2, "type");
    }
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
    type_specifier COLON identifier {
        SYMTYPE* type = try_find_type($1);
        try_add_symbol(type, $3, "parameter");
    }
    ;

dblock:
    L_BRACKET declaration_list R_BRACKET
    ;

declaration_list:
    declaration SEMI_COLON declaration_list
    | declaration
    ;

declaration:
    type_specifier {
        SYMTYPE* t = try_find_type($1);
        if(!t) {
            type_not_found_error($1);
        }
        push_context(t);
    } COLON identifier_list {
        pop_context();
    }
    ;


identifier_list:
    identifier assign_op constant COMMA identifier_list {
        SYMTYPE* type = (SYMTYPE*) peek_context();
        try_add_symbol(type, $1, "local");
    }
    | identifier assign_op constant {
        SYMTYPE* type = (SYMTYPE*) peek_context();
        try_add_symbol(type, $1, "local");
    }
    | identifier COMMA identifier_list {
        SYMTYPE* type = (SYMTYPE*) peek_context();
        try_add_symbol(type, $1, "local");
    }
    | identifier {
        SYMTYPE* type = (SYMTYPE*) peek_context();
        try_add_symbol(type, $1, "local");
    }
    ;

sblock:
    L_BRACE open_scope dblock statement_list close_scope R_BRACE
    | L_BRACE open_scope statement_list close_scope R_BRACE
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
    identifier
    | T_BOOLEAN
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


static SCOPE* symbols;
static SYMTYPE* types;
static ERROR* errors;

extern int get_row();
extern int get_column();

/*
    Accessors for parts of the program
    Makes compiling the compiler a bit easier
*/
SCOPE** get_symbol_table() {
    return &symbols;
}

ERROR** get_errors() {
    return &errors;
}

/*
    Simple stack to pass information between productions
*/
struct context {
    void* node;
    struct context* next;
} * context_stack = NULL;

void push_context(void* context) {
    struct context * c = malloc(sizeof(struct context));
    c->node = context;
    c->next = context_stack;
    context_stack = c;
}

void* pop_context() {
    
    if(!context_stack) {
        return NULL;
    }

    struct context * c = context_stack;
    void* r = c->node;
    context_stack = c->next;
    free(c);

    return r;
}

void* peek_context() {
    if(!context_stack) {
        return NULL;
    }
    return context_stack->node;
}

SCOPE* open_scope() {
    symbols = new_scope(symbols);
    return symbols;
}

SCOPE* close_scope() {
    symbols =  exit_scope(symbols);
    return symbols;
}

void initialize_types() {
    try_add_type(T_STRING, "string");
    try_add_type(T_REAL, "real");
    try_add_type(T_BOOLEAN, "Boolean");
    try_add_type(T_INTEGER, "integer");
    try_add_type(T_CHARACTER, "character");
}

SYMTYPE * try_add_type(int type, char* name) {
    
    if (find_type(types, name)) {
        return NULL;
    }
    
    types = add_type(types, type, name);
    return types;
}

SYMTYPE * try_find_type(char* name) {

    SYMTYPE * t = find_type(types, name);

    if(!t) {
        return NULL;
    }

    return t;
}

int try_add_symbol(SYMTYPE* type, char* name, char* ext) {
    
    if(find_entry(symbols->symbols, name)) {
        yyerror("Type redfinition:");
        printf("%s already exists in this scope.\n", name);
        return 0;
    }
    
    add_entry(symbols, type, name, ext);

    return 1;
}


void type_not_found_error(char* type) {
    static const char format[] = "\nERROR: %s, used here as a type, has not been declared at this point in the program.\n";

    char dest[strlen(format) + strlen(type) + 1];

    sprintf(dest, format, type);
    errors = push_error(errors, dest);
}

void yyerror (char *s) {

 }
