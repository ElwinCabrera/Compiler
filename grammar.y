%{
#include <stdio.h>
#include <string.h>
#include "stack.h"
#include "symbol_table.h"
#include "types.h"
#include "address.h"
#include "assignable.h"
#include "expression.h"
#include "intermediate_code.h"
#include "linked_list.h"

static int scope_counter = 0;
static STACK* errors;
static int yyerrstatus;
SYMBOL_TABLE* symbols;
INTERMEDIATE_CODE* code_table;
TYPE_CONTAINER* types;

STACK* function_context = 0;
STACK* symbol_context = 0;
STACK* case_context = 0;

extern int get_row();
extern int get_column();
int yylex();


/*
    Error Handling
*/
void yyerror(char*);
void fatal_error(char*);
void redefinition_error(char*,char*);
void syntax_error(char*);
void invalid_unary_expression(int, char*);
void invalid_binary_expression(int, char*, char*);
void type_mismatch_error(char*, char*);
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
    Wrappers to access types in static type linked list
*/
SYMTYPE* new_type(int, char*, int);
SYMTYPE* lookup_type(char*);
SYMBOL* find_symbol(char*);
SYMBOL* insert_new_symbol(SYMTYPE*, char*, int, char*);

%}

%start program

%union {
    int integer; 
    int boolean;
    double real;
    char character;
    char* string;
    struct scope* scope;
    struct linked_list* list;
    struct symtype* type;
    struct stack* stack;
    struct address* address;
    struct assignable* assignable;
    struct expression* expression;
}

%token <string> ID;
%token <integer> C_INTEGER;
%token <real> C_REAL;
%token <boolean> C_TRUE;
%token <boolean> C_FALSE;
%token <character> C_CHARACTER;
%token <string> C_STRING;

%type <string> identifier;
%type <integer> post_unary_operator pre_unary_operator mem_op;
%type <scope> open_scope close_scope sblock pblock;
%type <type> type_specifier;
%type <list> dblock declaration_list declaration identifier_list;
%type <list> parameter_list non_empty_parameter_list parameter_declaration;
%type <expression> expression;
%type <address> case constant;
%type <assignable> assignable;
%type <integer> next_instruction;
%type <stack> ablock argument_list non_empty_argument_list case_list;

%left L_PARENTHESIS R_PARENTHESIS
%left IS_NULL;
%left MUL DIV REM;
%left AND OR EQUAL_TO LESS_THAN;
%left ADD SUB_OR_NEG;
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

%token <integer> NULL_PTR
%token <integer> RESERVE
%token <integer> RELEASE

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
next_instruction: { $$ = code_table->next_instruction; } ;

program: 
    { 
        initialize_structs(); 
        add_code(code_table, new_tac(I_GOTO, NULL, NULL, NULL));
        add_code(code_table, new_tac(I_NOP, NULL, NULL, NULL));
    } open_scope definition_list next_instruction sblock {
        backpatch(code_table, 0, $4);
        add_code(code_table, new_tac(I_GOTO, NULL, NULL, label_address(1)));
        reorder_symbols($2);
    }
    ;

definition_list: 
    /* Empty String */
    | definition definition_list
    ;

definition:
    check_type_literal identifier COLON constant check_arrow type_specifier COLON L_PARENTHESIS constant check_r_parenthesis {
        SYMTYPE* type = new_type(MT_ARRAY, $2, 4);

        if(type) {
            type->element_type = $6;

            if($4->meta != AT_INT) { 
                type_mismatch_error("constant integer", $4->type ? $4->type->name : "NULL");
            } else {
                type->dimensions = $4->value.integer;
            }
        }

        insert_new_symbol(type, $2, ST_TYPE, "atype");
    }
    | check_type_literal identifier COLON constant check_arrow type_specifier {
        SYMTYPE* type = new_type(MT_ARRAY, $2, 4);
        if(type) {
            type->element_type = $6;
            if($4->meta != AT_INT) { 
                type_mismatch_error("constant integer", $4->type ? $4->type->name : "NULL");
            } else {
                type->dimensions = $4->value.integer;
            }
        }

        insert_new_symbol(type, $2, ST_TYPE, "atype");
    }
    | check_type_literal identifier COLON pblock check_arrow type_specifier {
        SYMTYPE* type = new_type(MT_FUNCTION, $2, 4);
        if(type) {
            type->parameters = $4;
            type->ret = new_symbol($6, $2, ST_RETURN, "local");
        }
        insert_new_symbol(type, $2, ST_TYPE, "ftype");
    }
    | FUNCTION identifier COLON type_specifier next_instruction {
        SYMBOL* s = insert_new_symbol($4, $2, ST_FUNCTION, "function");
        function_context = stack_push(function_context, s);
        s->label = label_address($5);
    } sblock { 
        ADDRESS* a = symbol_address($4->ret);
        add_code(code_table, new_tac(I_RETURN, NULL, NULL, a));
        function_context = stack_pop(function_context); 
    }
    | check_type_literal identifier COLON open_scope {
        new_type(MT_RECORD, $2, 4);
    } dblock close_scope  {
        SYMTYPE* t = lookup_type($2);
        add_symbols_to_scope($4, $6);
        reorder_symbols($4);
        t->members = $4;
        insert_new_symbol(t, $2, ST_TYPE, "rtype");
    }
    ;

pblock:
    L_PARENTHESIS open_scope parameter_list close_scope check_r_parenthesis {
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
        $$ = ll_combine($3, $1);
    }
    | parameter_declaration
    ;

parameter_declaration:
    type_specifier COLON identifier {
        $$ = ll_new(new_symbol($1, $3, PARAMETER, "parameter"));
    }
    ;

dblock:
    check_l_bracket declaration_list check_r_bracket {
        $$ = $2;
    }
    ;

declaration_list:
    declaration SEMI_COLON declaration_list {
        $$ = ll_combine($1, $3);
    }
    | declaration
    ;

declaration:
    type_specifier {
        symbol_context = stack_push(symbol_context, $1);
    } COLON identifier_list {
        $$ = $4;
        symbol_context = stack_pop(symbol_context);
    }
    ;


identifier_list:
    identifier assign_op constant COMMA identifier_list {
        SYMTYPE* type = stack_peek(symbol_context);
        SYMBOL* s = new_symbol(type, $1, ST_LOCAL, "local");
        ADDRESS* a = symbol_address(s);
        handle_assignment(assignable_variable(a), const_expression($3));
        $$ = ll_insertfront($5, s);
    }
    | identifier assign_op constant {
        SYMTYPE* type = stack_peek(symbol_context);
        SYMBOL* s = new_symbol(type, $1, ST_LOCAL, "local");
        ADDRESS* a = symbol_address(s);
        handle_assignment(assignable_variable(a), const_expression($3));
        $$ = ll_new(new_symbol(type, $1, ST_LOCAL, "local"));
    }
    | identifier COMMA identifier_list {
        SYMTYPE* type = stack_peek(symbol_context);
        $$ = ll_insertfront($3, new_symbol(type, $1, ST_LOCAL, "local"));

    }
    | identifier {
        SYMTYPE* type = stack_peek(symbol_context);
        $$ = ll_new(new_symbol(type, $1, ST_LOCAL, "local"));
    }
    ;

sblock:
    L_BRACE open_scope dblock {
        add_symbols_to_scope($2, $3);
        reorder_symbols($2);
    } statement_list close_scope check_r_brace {
        $$ = $2;
    }
    | L_BRACE open_scope statement_list close_scope check_r_brace {
        $$ = $2;
    }
    ;
    
statement_list:
    statement statement_list;
    | statement;

statement:
    FOR L_PARENTHESIS statement SEMI_COLON next_instruction expression next_instruction {
//  1   2             3         4          5                6          7                8
        ADDRESS* a = exp_rvalue($6);
        SYMTYPE* type = a ? a->type : NULL;
        if(!check_typename(type, "Boolean")) {
            type_mismatch_error("Boolean", type ? type->name : "NULL");
        }
        add_code(code_table, new_tac(I_TEST_FALSE, a, NULL, NULL));
    } next_instruction {
//    9                10
        add_code(code_table, new_tac(I_GOTO, NULL, NULL, NULL));
    } SEMI_COLON next_instruction statement {
//    11         12               13        14
        add_code(code_table, new_tac(I_GOTO, NULL, NULL, label_address($5)));
    } R_PARENTHESIS next_instruction sblock {
//    15            16               17     18
        backpatch(code_table, $9, $16);
        add_code(code_table, new_tac(I_GOTO, NULL, NULL, label_address($12)));
    } next_instruction {
//    19               20
        backpatch(code_table, $7, $19);
    }
    | SWITCH L_PARENTHESIS expression check_r_parenthesis {
//    1      2             3          4                   5
        ADDRESS* a = exp_rvalue($3);
        SYMTYPE* type = a ? a->type : NULL;
        if(!check_typename(type, "integer")) {
            type_mismatch_error("integer", type ? type->name : "NULL");
        }
        case_context = stack_push(case_context, $3);
    } case_list OTHERWISE COLON next_instruction sblock next_instruction {
//    6         7         8     9                 10     11
        STACK* s = $6;
        while(s) {
            ADDRESS* a = stack_peek(s);
            backpatch(code_table, a->value.label, $11);
            s = stack_pop(s);
        }
        case_context = stack_pop(case_context);
    }
    | IF L_PARENTHESIS expression next_instruction { 
//    1  2             3          4                5
        ADDRESS* a = exp_rvalue($3);
        SYMTYPE* type = a ? a->type : NULL;
        if(!check_typename(type, "Boolean")) {
            type_mismatch_error("Boolean", type ? type->name : "NULL");
        }
        add_code(code_table, new_tac(I_TEST_FALSE, a, NULL, NULL));
    } check_r_parenthesis THEN sblock next_instruction {
//    6                   7    8      9                10
        add_code(code_table, new_tac(I_GOTO, NULL, NULL, NULL));
    } next_instruction ELSE sblock next_instruction {
//    11               12   13     14
        backpatch(code_table, $4, $11);
        backpatch(code_table, $9, $14);
    }
    | WHILE L_PARENTHESIS next_instruction expression next_instruction {
//    1     2             3                4          5                6
        ADDRESS* a = exp_rvalue($4);
        SYMTYPE* type = a ? a->type : NULL;
        if(!check_typename(type, "Boolean")) {
            type_mismatch_error("Boolean", type ? type->name : "NULL");
        }
        add_code(code_table, new_tac(I_TEST_FALSE, a, NULL, NULL));
    } check_r_parenthesis sblock {
//    7                   8      9
        add_code(code_table, new_tac(I_GOTO, NULL, NULL, label_address($3)));
    } next_instruction {
//    10               11
        backpatch(code_table, $5, $10);
    }
    | assignable assign_op expression semi_colon_after_statement {
        handle_assignment($1, $3);
    }
    | mem_op assignable semi_colon_after_statement {
        handle_memop($1, $2);
    }
    | sblock
    ;

case_list:
    case case_list {
        $$ = stack_push($2, $1);
    }
    | case {
        $$ = stack_push(NULL, $1);
    }
    ;

case:
    CASE constant next_instruction {
//  1    2        3                4
        EXPRESSION* exp = stack_peek(case_context);
        SYMTYPE* e_type = expression_type(exp);
        if(!check_typename(e_type, "integer")) {
            type_mismatch_error("integer", e_type ? e_type->name : "NULL");
        }
        add_code(code_table, new_tac(I_TEST_NOTEQUAL, exp_rvalue(exp), $2, NULL));
    } COLON sblock next_instruction {
//    5     6      7
        $$ = label_address($7);
        add_code(code_table, new_tac(I_GOTO, NULL, NULL, NULL));
        backpatch(code_table, $3, $7 + 1);
    }
    ;

assignable:
    identifier { 
        SYMBOL* s = NULL;
        SYMBOL* fn = stack_peek(function_context);

        if(fn) {
            if(strcmp(fn->name, $1) == 0) {
                s = fn;
            } else {
                s = find_entry(fn->type->parameters->symbols, $1);
            }
        }

        if(!s) {
            s = find_symbol($1); 
        }

        if (!s) {
            symbol_not_found_error($1, "variable");
        } else if (s->meta == TYPE) {
            type_as_var_error(s->name);
        } else {
            $$ = assignable_variable(symbol_address(s));
        }
    }
    | assignable rec_op identifier {
        ADDRESS* a = assignable_rvalue($1);
        
        if(a) {
            if(!check_metatype(a->type, MT_RECORD)) {
                incorrect_type_error(a->value.symbol->name, "record");
                $$ = NULL;
            } else {
                SYMBOL* s = find_entry(a->value.symbol->type->members->symbols, $3); 
                if(!s) {
                    symbol_not_found_error($3, "record member");
                    $$ = NULL;
                } else {
                    $$ = assignable_record(a, symbol_address(s));
                }
            } 
        } else {
            $$ = NULL;
        }
    }
    | assignable ablock {
        ADDRESS* a = assignable_rvalue($1);
        if(!a) {
            $$ = NULL;
        } else if(check_metatype(a->type, MT_FUNCTION)) {
            $$ = assignable_function(a, $2);
        } else if(check_metatype(a->type, MT_ARRAY)) {
            $$ = assignable_array(a, $2);
        } else {
            incorrect_type_error(a->value.symbol->name, "array or function");
        }
    }
    ;

ablock:
    L_PARENTHESIS argument_list check_r_parenthesis {
        $$ = $2;
    }
    ;

argument_list:
    /* Empty String */ { $$ = NULL; }
    | non_empty_argument_list;

non_empty_argument_list:
    expression COMMA non_empty_argument_list {
        $$ = stack_push($3, $1);
    }
    | expression {
        $$ = stack_push(NULL, $1);
    }
    ;

/* 
    The binary operators are all separate because bison
    doesn't properly handle precedence if they're all reduced to
    the same token. 
 */

expression:
    expression ADD expression { $$ = binary_expression($2, $1, $3); }
    | expression SUB_OR_NEG expression { $$ = binary_expression($2, $1, $3); }
    | expression MUL expression { $$ = binary_expression($2, $1, $3); }
    | expression DIV expression { $$ = binary_expression($2, $1, $3); }
    | expression REM expression { $$ = binary_expression($2, $1, $3); }
    | expression AND next_instruction {
        TAC* short_circuit = new_tac(I_TEST_FALSE, exp_rvalue($1), NULL, NULL);
        add_code(code_table, short_circuit);
    } expression {
        ADDRESS* a = temp_address(lookup_type("Boolean"));
        ASSIGNABLE* assign = assignable_variable(a);
        EXPRESSION* false_const = const_expression(boolean_address(0));
        handle_assignment(assign, $5);
        add_code(code_table, new_tac(I_GOTO, NULL, NULL, label_address(code_table->next_instruction + 2)));
        backpatch(code_table, $3, code_table->next_instruction);
        handle_assignment(assign, false_const);
        $$ = temp_expression(a);
    }
    | expression OR next_instruction {
        TAC* short_circuit = new_tac(I_TEST, exp_rvalue($1), NULL, NULL);
        add_code(code_table, short_circuit);
    } expression { 
        ADDRESS* a = temp_address(lookup_type("Boolean"));
        ASSIGNABLE* assign = assignable_variable(a);
        EXPRESSION* true_const = const_expression(boolean_address(1));
        handle_assignment(assign, $5);
        add_code(code_table, new_tac(I_GOTO, NULL, NULL, label_address(code_table->next_instruction + 2)));
        backpatch(code_table, $3, code_table->next_instruction);
        handle_assignment(assign, true_const);
        $$ = temp_expression(a);
    }
    | expression LESS_THAN expression { $$ = binary_expression($2, $1, $3); }
    | expression EQUAL_TO expression { $$ = binary_expression($2, $1, $3); }
    | expression post_unary_operator {
        $$ = unary_expression($2, $1);
    }
    | pre_unary_operator expression %prec pre_unary_prec {
        $$ = unary_expression($1, $2);
    }
    | L_PARENTHESIS expression R_PARENTHESIS {
        $$ = $2;
    }
    | constant {
        $$ = const_expression($1);
    }
    | assignable {
        $$ = assignable_expression($1);
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
    C_INTEGER               { $$ = int_address($1); }
    | C_REAL                { $$ = real_address($1); }
    | C_CHARACTER           { $$ = char_address($1); }
    | C_STRING              { $$ = string_address($1); }
    | C_TRUE                { $$ = boolean_address($1); }
    | C_FALSE               { $$ = boolean_address($1); }
    | NULL_PTR              { $$ = null_address(); }
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

/* Syntax Errors */

check_type_literal:
    TYPE
    | error {
        syntax_error("Type declarations must begin with \"type\"");
    }
    ;

semi_colon_after_statement:
    SEMI_COLON
    | error {
        syntax_error("; expected following a statement");
    }
    ;

check_r_parenthesis:
    R_PARENTHESIS
    | error {
        syntax_error(") expected to match opening (");
    }
    ;

check_l_bracket:
    L_BRACKET
    | error {
        syntax_error("[ expected to open declaration block");
    }
    ;

check_r_bracket:
    R_BRACKET
    | error {
        syntax_error("] expected to match opening [");
    }
    ;

check_r_brace:
    R_BRACE
    | error {
        syntax_error("} expected to match opening {");
    }
    ;

check_arrow:
    ARROW
    | error {
        syntax_error("-> expected");
    }
    ;

%%


/*
    Accessors for parts of the program
    Makes compiling the compiler a bit easier
*/

STACK** get_errors() {
    return &errors;
}

SCOPE* open_scope() {
    symbols->current_scope = new_scope(symbols->current_scope, scope_counter++);
    return symbols->current_scope;
}

SCOPE* close_scope() {
    symbols->current_scope = exit_scope(symbols->current_scope);
    return symbols->current_scope;
}

SYMBOL* insert_new_symbol(SYMTYPE* type, char* name, int meta, char* extra) {
    SYMBOL* s = new_symbol(type, name, meta, extra);
    add_symbols_to_scope(symbols->current_scope, ll_new(s));
    return s;
}

void initialize_structs() {
    code_table = get_intermediate_code();
    types = get_type_container();
    symbols = get_symbol_table();

    new_type(MT_PRIMITIVE, "string", 4);
    new_type(MT_PRIMITIVE, "real", 8);
    new_type(MT_PRIMITIVE, "Boolean", 1);
    new_type(MT_PRIMITIVE, "integer", 4);
    new_type(MT_PRIMITIVE, "character", 1);
    new_type(MT_PRIMITIVE, "nullconst", 4);
}

SYMTYPE* new_type(int type, char* name, int width) {
    SYMTYPE* t = add_type(types, type, name, width);
    if(!t){
        redefinition_error("type", name); 
    }
    return t;
}

SYMTYPE* lookup_type(char* name) {
    return find_type(types, name);
}

SYMBOL* find_symbol(char* name) {
    return find_in_scope(symbols->current_scope, name);
}

/*
    Error Handling
*/

void notify_error(char* e) {
    yyerror(e);
    errors = stack_push(errors, strdup(e));
    yyerrok;
}

void fatal_error(char* err) {
    printf("LINE %d:%d - [FATAL ERROR] %s\n", get_row(), get_column(), err);
    yyerror(NULL);
}

void argument_count_mismatch(int expected, int actual) {
    const char format[] = "LINE %d:%d - ERROR: Incorrect argument count (Expected: %d, Actual: %d)\n";
    
    char dest[strlen(format) + 26];

    sprintf(dest, format, get_row(), get_column(), expected, actual);
    notify_error(dest);
}

void invalid_unary_expression(int op, char* t1) {
    const char format[] = "LINE %d:%d - ERROR: unary operator '%s' cannot be applied to type '%s'\n";
    
    char dest[strlen(format) + (t1 ? strlen(t1) : 0) + 26];

    sprintf(dest, format, get_row(), get_column(), get_op_string(op) , t1);
    notify_error(dest);
}

void invalid_binary_expression(int op, char* t1, char* t2) {
    const char format[] = "LINE %d:%d - ERROR: binary operator '%s' cannot be applied to type '%s' and '%s'\n";
    
    char dest[strlen(format) + (t1 ? strlen(t1) : 0) + (t2 ? strlen(t2) : 0) + 26];

    sprintf(dest, format, get_row(), get_column(), get_op_string(op), t2, t1);
    notify_error(dest);
}

void redefinition_error(char* t1, char* t2) {
    const char format[] = "LINE %d:%d - ERROR: %s redinition; '%s' already has meaning'\n";
    
    char dest[strlen(format) + (t1 ? strlen(t1) : 0) + (t2 ? strlen(t2) : 0) + 21];

    sprintf(dest, format, get_row(), get_column(), t1, t2);
    notify_error(dest);
}

void type_mismatch_error(char* t1, char* t2) {
    const char format[] = "LINE %d:%d - ERROR: type '%s' is incompatible with expected type '%s'\n";
    
    char dest[strlen(format) + (t1 ? strlen(t1) : 0) + (t2 ? strlen(t2) : 0) + 21];

    sprintf(dest, format, get_row(), get_column(), t2, t1);
    notify_error(dest);
}

void type_as_var_error(char* name) {
    const char format[] = "LINE %d:%d - ERROR: %s, a type, is used here as a variable\n";
    
    char dest[strlen(format) + strlen(name) + 21];

    sprintf(dest, format, get_row(), get_column(), name);
    notify_error(dest);
}

void incorrect_type_error(char* name, char* type) {
    const char format[] = "LINE %d:%d - ERROR: %s is not of type %s\n";
    
    char dest[strlen(format) + strlen(name) + strlen(type)  + 21];

    sprintf(dest, format, get_row(), get_column(), name, type);
    notify_error(dest);
}

void symbol_not_found_error(char* name, char* style) {
    const char format[] = "LINE %d:%d - ERROR: %s, used here as a %s, has not been declared at this point in the program.\n";

    char dest[strlen(format) + strlen(name) + strlen(style)  + 21];

    sprintf(dest, format, get_row(), get_column(), name, style);
    
    notify_error(dest);
}

void syntax_error(char* expected) {
    const char format[] = "LINE %d:%d - ERROR: %s\n";
    
    char dest[strlen(format) + strlen(expected) + 21];

    sprintf(dest, format, get_row(), get_column(), expected);
    notify_error(dest);
}

void yyerror (char *s) {

 }
