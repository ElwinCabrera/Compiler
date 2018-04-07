%{
#include <stdio.h>
#include <string.h>
#include "symbolTable.h"
#include "types.h"
#include "errors.h"
#include "node.h"
#include "ir.h"

static int scope_counter = 0;
static SCOPE* symbols;
static SYMTYPE* types;
static ERROR* errors;
static IRTABLE* code_table;
static int yyerrstatus;

extern int get_row();
extern int get_column();
int yylex();

/*
    Simple stack to pass information between productions
*/
struct context {
    void* node;
    struct context* next;
} * context_stack = NULL;


/*
    Error Handling
*/
void yyerror(char*);
void symbol_not_found_error(char*, char*);
void incorrect_type_error(char*, char*);
void type_as_var_error(char*);

/*
    Setup scope, IR table, and primitive types
*/
void initialize_structs();

/*
    Used by rule actions for manipulating and accessing scope
*/
SCOPE* open_scope();
SCOPE* close_scope();

/*
    Helpers for passing type or expression information
*/
void push_context(void*);
void* pop_context();
void* peek_context();

/*
    Wrappers to access types in static type linked list
*/
SYMTYPE* new_type(int, char*);
SYMTYPE* lookup_type(char*);

SYMTAB* find_symbol(char*);
void insert_new_symbol(SYMTYPE*, char*, int, char*);


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
    struct symtype* type;
    struct node* node;
}

%token <string> ID;
%token <integer> C_INTEGER;
%token <real> C_REAL;
%token <boolean> C_TRUE;
%token <boolean> C_FALSE;
%token <character> C_CHARACTER;
%token <string> C_STRING;

%type <string> identifier;
%type <integer> binary_operator post_unary_operator pre_unary_operator;
%type <scope> open_scope close_scope sblock pblock;
%type <type> type_specifier;
%type <symbol> dblock declaration_list declaration;
%type <symbol> parameter_list non_empty_parameter_list parameter_declaration;
%type <symbol> identifier_list;
%type <node> constant expression assignable;

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

%token <integer> ADD
%token <integer> SUB_OR_NEG
%token <integer> MUL
%token <integer> DIV
%token <integer> REM
%token <integer> DOT
%token <integer> LESS_THAN
%token <integer> EQUAL_TO
%token <integer> ASSIGN
%token <integer> INT2REAL
%token <integer> REAL2INT
%token <integer> IS_NULL
%token <integer> NOT
%token <integer> AND
%token <integer> OR

%token COMMENT;

%%

open_scope: { $$ = open_scope(); } ;
close_scope: { $$ = close_scope(); } ;

program: 
    { initialize_structs(); } definition_list sblock
    ;

definition_list: 
    /* Empty String */
    | definition definition_list
    ;

definition:
    TYPE identifier COLON constant ARROW type_specifier COLON L_PARENTHESIS constant R_PARENTHESIS {
        SYMTYPE* type = new_type(MT_ARRAY, $2);
        insert_new_symbol(type, $2, TYPE, "atype");
    }
    | TYPE identifier COLON constant ARROW type_specifier {
        SYMTYPE* type = new_type(MT_ARRAY, $2);
        insert_new_symbol(type, $2, TYPE, "atype");
    }
    | TYPE identifier COLON pblock ARROW type_specifier {
        SYMTYPE* type = new_type(MT_FUNCTION, $2);
        type->details.function->parameters = $4;
        type->details.function->return_type = $6;
        insert_new_symbol(type, $2, TYPE, "ftype");
    }
    | FUNCTION identifier COLON type_specifier {
        insert_new_symbol($4, $2, FUNCTION, "function");
        push_context($4);
    } sblock { pop_context(); }
    | TYPE identifier COLON open_scope {
        new_type(MT_RECORD, $2);
    } dblock close_scope  {
        SYMTYPE* t = lookup_type($2);
        add_symbols_to_scope($4, $6);
        t->details.record->members = $4;
        insert_new_symbol(t, $2, TYPE, "rtype");
    }
    ;

pblock:
    L_PARENTHESIS open_scope parameter_list close_scope R_PARENTHESIS {
        add_symbols_to_scope($2, $3);
        $$ = $2;
    }
    ;

parameter_list:
    /* Empty String */ { $$ = NULL; }
    | non_empty_parameter_list 
    ;

non_empty_parameter_list:
    parameter_declaration COMMA non_empty_parameter_list {
        $$ = add_symbols($1, $3);
    }
    | parameter_declaration
    ;

parameter_declaration:
    type_specifier COLON identifier {
        $$ = new_symbol($1, $3, PARAMETER, "parameter");
    }
    ;

dblock:
    L_BRACKET declaration_list R_BRACKET {
        $$ = $2;
    }
    ;

declaration_list:
    declaration SEMI_COLON declaration_list {
        $$ = add_symbols($1, $3);
    }
    | declaration
    ;

declaration:
    type_specifier {
        push_context($1);
    } COLON identifier_list {
        $$ = $4;
        pop_context();
    }
    ;


identifier_list:
    identifier assign_op constant COMMA identifier_list {
        SYMTYPE* type = (SYMTYPE*) peek_context();
        $$ = add_symbols($5, new_symbol(type, $1, LOCAL, "local"));
    }
    | identifier assign_op constant {
        SYMTYPE* type = (SYMTYPE*) peek_context();
        $$ = new_symbol(type, $1, LOCAL, "local");
    }
    | identifier COMMA identifier_list {
        SYMTYPE* type = (SYMTYPE*) peek_context();
        $$ = add_symbols($3, new_symbol(type, $1, LOCAL, "local"));

    }
    | identifier {
        SYMTYPE* type = (SYMTYPE*) peek_context();
        $$ = new_symbol(type, $1, LOCAL, "local");
    }
    ;

sblock:
    L_BRACE open_scope dblock {
        add_symbols_to_scope($2, $3);
    } statement_list close_scope R_BRACE {
        $$ = $2;
    }
    | L_BRACE open_scope statement_list close_scope R_BRACE {
        $$ = $2;
    }
    ;
    
statement_list:
    statement statement_list;
    | statement;

statement:
    FOR L_PARENTHESIS statement SEMI_COLON expression SEMI_COLON statement R_PARENTHESIS sblock
    | SWITCH L_PARENTHESIS expression R_PARENTHESIS case_list OTHERWISE COLON sblock
    | IF L_PARENTHESIS expression R_PARENTHESIS THEN sblock ELSE sblock
    | WHILE L_PARENTHESIS expression R_PARENTHESIS sblock
    | assignable assign_op expression SEMI_COLON {
        if(!type_check_assignment($1, $3)) {
            //printf("Type mismatch error\n");
        } else {
            NODE* n = add_instruction(code_table, ID, $1, NULL);
            add_instruction(code_table, ASSIGN, n, $3);
        }
    }
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
    identifier { 
        SYMTAB* s = find_symbol($1); 
        if (!s) {
            SYMTYPE* st = peek_context();
            if(st && st->meta == MT_FUNCTION) {
                s = find_entry(st->details.function->parameters->symbols, $1);
            }
            if(!s) {
                symbol_not_found_error($1, "variable");
            }
        } else if (s->meta == TYPE) {
            type_as_var_error(s->name);
        }
        $$ = symbol_node(s);
    }
    | assignable rec_op identifier {
        if($1 && $1->value.symbol) {
            if(!check_metatype($1->value.symbol->type, MT_RECORD)) {
                incorrect_type_error($1->value.symbol->name, "record");
                $$ = NULL;
            } else {
                SYMTAB* s = find_entry($1->value.symbol->type->details.record->members->symbols, $3); 
                if(!s) {
                    symbol_not_found_error($3, "record member");
                    $$ = NULL;
                } else {
                    $$ = symbol_node(s);
                }
            } 
        } else {
            $$ = NULL;
        }
    }
    | assignable ablock {
        if($1 && $1->value.symbol) {
            if(check_metatype($1->value.symbol->type, MT_FUNCTION)) {
                /* 
                    TODO: FUNCTION CALL
                */
                // printf("Function call: %s\n", $1->name);
            } else if(check_metatype($1->value.symbol->type, MT_ARRAY)) {
                /* 
                    TODO: ARRAY ACCESS
                */
                // printf("Array access: %s", $1->name);
            } else {
                incorrect_type_error($1->value.symbol->name, "array or function");
            }
        } else {

        } 
    }
    ;

ablock:
    L_PARENTHESIS argument_list R_PARENTHESIS {
        /*
            TODO: When an argument list is defined, uncomment,
            and add a $$ = NULL; rule for the empty case below
            $$ = $2;
        */
    }
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
        $$ = add_instruction(code_table, $2, $1, $3);
    }
    | expression post_unary_operator {
        $$ = add_instruction(code_table, $2, $1, NULL);
    }
    | pre_unary_operator expression %prec pre_unary_prec {
        $$ = add_instruction(code_table, $1, $2, NULL);
    }
    | L_PARENTHESIS expression R_PARENTHESIS {
        $$ = $2;
    }
    | constant 
    | assignable {
        $$ = add_instruction(code_table, ID, $1, NULL);
    }
    ;


identifier:
    ID
    ;

type_specifier:
    identifier { 
        SYMTYPE * t = lookup_type($1);
        if(!t) {
            symbol_not_found_error($1, "type");
        }
        $$ = t;
    }
    | T_BOOLEAN             { $$ = lookup_type($1); }
    | T_CHARACTER           { $$ = lookup_type($1); }
    | T_INTEGER             { $$ = lookup_type($1); }
    | T_REAL                { $$ = lookup_type($1); }
    | T_STRING              { $$ = lookup_type($1); }
    ;

constant:
    C_INTEGER               { $$ = int_node($1); }
    | C_REAL                { $$ = real_node($1); }
    | C_CHARACTER           { $$ = char_node($1); }
    | C_STRING              { $$ = string_node($1); }
    | C_TRUE                { $$ = boolean_node($1); }
    | C_FALSE               { $$ = boolean_node($1); }
    | NULL_PTR              { $$ = null_node(); }
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
    symbols = new_scope(symbols, scope_counter++);
    return symbols;
}

SCOPE* close_scope() {
    symbols =  exit_scope(symbols);
    return symbols;
}

void insert_new_symbol(SYMTYPE* type, char* name, int meta, char* extra) {
    SYMTAB* s = new_symbol(type, name, meta, extra);
    add_symbols_to_scope(symbols, s);
}

void initialize_structs() {
    open_scope();
    code_table = new_ir_table(1000);

    new_type(MT_PRIMITIVE, "string");
    new_type(MT_PRIMITIVE, "real");
    new_type(MT_PRIMITIVE, "Boolean");
    new_type(MT_PRIMITIVE, "integer");
    new_type(MT_PRIMITIVE, "character");
}

SYMTYPE* new_type(int type, char* name) {
    
    if (find_type(types, name)) {
        return NULL;
    }
    
    types = add_type(types, type, name);
    return types;
}

SYMTYPE* lookup_type(char* name) {

    SYMTYPE * t = find_type(types, name);

    if(!t) {
        return NULL;
    }

    return t;
}

SYMTAB* find_symbol(char* name) {
    return find_in_scope(symbols, name);
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
