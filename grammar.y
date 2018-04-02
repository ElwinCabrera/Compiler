%{
#include <stdio.h>
#include <string.h>
#include "symbolTable.h"
#include "types.h"
#include "errors.h"
#include "expressions.h"

int yylex();

/*
    Error Handling
*/
void yyerror(char*);
void symbol_not_found_error(char*, char*);
void incorrect_type_error(char*, char*);
void type_as_var_error(char*);

/*
    Used by rule actions for manipulating and accessing scope
*/
SCOPE* open_scope();
SCOPE* close_scope();
SCOPE* insert_scope(SCOPE*);

/*
    Helpers for passing type or expression information
*/
void push_context(void*);
void* pop_context();
void* peek_context();

/*
    Wrappers to access types in static type linked list
*/
SYMTYPE* try_add_type(int, char*);
SYMTYPE* try_find_type(char*);

/*
    Adds primitive types to the type linked list
*/
void initialize_types();

/*
    Wrappers to access types in static symbol table 
*/
int try_add_symbol(SYMTYPE*, char*, int, char*);
SYMTAB* try_find_symbol(char*);

/*
    Expressions
*/

EXPR* try_expression(void*);
EXPR* try_binary_expression(void*, char*, void*);
EXPR* try_post_unary_expression(void*, char*);
EXPR* try_pre_unary_expression(char*, void*);

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
    int integer; 
    int boolean;
    double real;
    char character;
    char* string;
    struct scope* scope;
    struct symtab* symbol;
    struct expr* expression;
    struct Node* node;
}

%token <string> ID;
%token <integer> C_INTEGER;
%token <real> C_REAL;
%token <boolean> C_TRUE;
%token <boolean> C_FALSE;
%token <character> C_CHARACTER;
%token <string> C_STRING;

%type <string> identifier type_specifier;
%type <string> binary_operator post_unary_operator pre_unary_operator;
%type <scope> open_scope close_scope;
%type <symbol> assignable;
%type <expression> expression;
%type <node> constant;

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
%token PARAMETER
%token LOCAL
%token RETURN

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

%token <string> ADD
%token <string> SUB_OR_NEG
%token <string> MUL
%token <string> DIV
%token <string> REM
%token <string> DOT
%token <string> LESS_THAN
%token <string> EQUAL_TO
%token <string> ASSIGN
%token <string> INT2REAL
%token <string> REAL2INT
%token <string> IS_NULL
%token <string> NOT
%token <string> AND
%token <string> OR

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
        try_add_symbol(type, $2, TYPE, "type");
    }
    | TYPE identifier COLON constant ARROW type_specifier {
        SYMTYPE* type = try_add_type(ARRAY, $2);
        try_add_symbol(type, $2, TYPE, "type");
    }
    | TYPE identifier COLON open_scope pblock close_scope ARROW type_specifier {
        SYMTYPE* type = try_add_type(FUNCTION, $2);
        type->details.function->parameters = $4;
        SYMTYPE* return_type = try_find_type($8);
        if(!return_type) { 
            symbol_not_found_error($8, "type");
        }
        type->details.function->return_type = return_type;
        try_add_symbol(type, $2, TYPE, "type");
    }
    | FUNCTION identifier COLON type_specifier {
        SYMTYPE* type = try_find_type($4);  
        if(!type) {
            symbol_not_found_error($4, "type");
        } else {
            insert_scope(type->details.function->parameters);
            try_add_symbol(type->details.function->return_type, $2, RETURN, "return");
        }

    }  open_scope sblock close_scope close_scope {
        SYMTYPE* type = try_find_type($4);
        if(!type) {
            symbol_not_found_error($4, "type");
        } else {
            try_add_symbol(type, $2, FUNCTION, "function");
        }
    }
    | TYPE identifier COLON open_scope {
        try_add_type(RECORD, $2);
    } dblock close_scope  {
        SYMTYPE* type = try_find_type($2);
        type->details.record->members = $4;
        try_add_symbol(type, $2, TYPE, "type");
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
        try_add_symbol(type, $3, PARAMETER, "parameter");
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
            symbol_not_found_error($1, "type");
        }
        push_context(t);
    } COLON identifier_list {
        pop_context();
    }
    ;


identifier_list:
    identifier assign_op constant COMMA identifier_list {
        SYMTYPE* type = (SYMTYPE*) peek_context();
        try_add_symbol(type, $1, LOCAL, "local");
    }
    | identifier assign_op constant {
        SYMTYPE* type = (SYMTYPE*) peek_context();
        try_add_symbol(type, $1, LOCAL, "local");
    }
    | identifier COMMA identifier_list {
        SYMTYPE* type = (SYMTYPE*) peek_context();
        try_add_symbol(type, $1, LOCAL, "local");
    }
    | identifier {
        SYMTYPE* type = (SYMTYPE*) peek_context();
        try_add_symbol(type, $1, LOCAL, "local");
    }
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
    | open_scope sblock close_scope
    ;

case_list:
    case case_list
    | case
    ;

case:
    CASE constant COLON sblock
    ;

assignable:
    identifier { 
        SYMTAB* s = try_find_symbol($1); 
        if (!s) {
            symbol_not_found_error($1, "variable");
        } else if (s->meta == TYPE) {
            type_as_var_error(s->name);
        }
        $$ = s;
    }
    | assignable rec_op {
        
        if($1) {
            if(!check_type($1->type, RECORD, NULL)) {
                incorrect_type_error($1->name, "record");
            } 
        } 

    } identifier 
    | assignable ablock {

        if($1) {
            if(check_type($1->type, FUNCTION, NULL)) {
                /* 
                    TODO: FUNCTION CALL
                */
            } else if(check_type($1->type, ARRAY, NULL)) {
                /* 
                    TODO: ARRAY ACCESS
                */
            } else {
                incorrect_type_error($1->name, "array or function");
            }
        } 
    }
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
    expression binary_operator expression {
        $$ = try_binary_expression($1, $2, $3);
    }
    | expression post_unary_operator {
        $$ = try_post_unary_expression($1, $2);
    }
    | pre_unary_operator expression %prec pre_unary_prec {
        $$ = try_pre_unary_expression($1, $2);
    }
    | L_PARENTHESIS expression R_PARENTHESIS {
        $$ = $2;
    }
    | constant {
        $$ = try_expression($1);
    }
    | assignable {
        $$ = try_expression($1);
    }
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
    C_INTEGER             { $$ = NULL; }
    | C_REAL              { $$ = NULL; }
    | C_CHARACTER         { $$ = NULL; }
    | C_STRING            { $$ = NULL; }
    | C_TRUE              { $$ = NULL; }
    | C_FALSE             { $$ = NULL; }

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
static int yyerrstatus;

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

SCOPE* insert_scope(SCOPE* s) {
    symbols =  insert_and_enter_scope(s, symbols);
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

int try_add_symbol(SYMTYPE* type, char* name, int meta, char* ext) {
    
    if(!symbols) {
        return 0;
    }

    if(find_entry(symbols->symbols, name)) {
        // Symbol redefined in the same table
        return 0;
    }
    
    add_entry(symbols, type, name, meta, ext);

    return 1;
}

SYMTAB* try_find_symbol(char* name) {
    return find_in_scope(symbols, name);
}


EXPR* try_expression(void* n) {
    return NULL;
}

EXPR* try_binary_expression(void* lhs, char* op, void* rhs) {
    return NULL;
}

EXPR* try_post_unary_expression(void* lhs, char* op) {
    return NULL;
}

EXPR* try_pre_unary_expression(char* op, void* rhs) {
    return NULL;
}


/*
    Error Handling
*/
void type_as_var_error(char* name) {
    const char format[] = "LINE %d:%d - ERROR: %s, a type, is used here as a variable\n";
    
    char dest[strlen(format) + strlen(name) + 21];

    sprintf(dest, format, get_row(), get_column(), name);
    yyerror(dest);
    yyerrok;
}

void incorrect_type_error(char* name, char* type) {
    const char format[] = "LINE %d:%d - ERROR: %s is not of type %s\n";
    
    char dest[strlen(format) + strlen(name) + strlen(type)  + 21];

    sprintf(dest, format, get_row(), get_column(), name, type);
    yyerror(dest);
    yyerrok;
}

void symbol_not_found_error(char* name, char* style) {
    const char format[] = "LINE %d:%d - ERROR: %s, used here as a %s, has not been declared at this point in the program.\n";

    char dest[strlen(format) + strlen(name) + strlen(style)  + 21];

    sprintf(dest, format, get_row(), get_column(), name, style);
    
    yyerror(dest);
    yyerrok;
}

void yyerror (char *s) {
    errors = push_error(errors, s);
 }
