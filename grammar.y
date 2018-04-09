%{
#include <stdio.h>
#include <string.h>
#include "ir.h"
#include "node.h"
#include "stack.h"
#include "symbolTable.h"
#include "types.h"

static int scope_counter = 0;
static SCOPE* symbols;
static SYMTYPE* types;
static STACK* errors;
static IRTABLE* code_table;
static int yyerrstatus;

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
void syntax_error(char*);
void argument_count_mismatch(int, int);
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
    struct stack* stack;
}

%token <string> ID;
%token <integer> C_INTEGER;
%token <real> C_REAL;
%token <boolean> C_TRUE;
%token <boolean> C_FALSE;
%token <character> C_CHARACTER;
%token <string> C_STRING;

%type <string> identifier;
%type <integer> binary_operator post_unary_operator pre_unary_operator mem_op;
%type <scope> open_scope close_scope sblock pblock;
%type <type> type_specifier;
%type <symbol> dblock declaration_list declaration;
%type <symbol> parameter_list non_empty_parameter_list parameter_declaration;
%type <symbol> identifier_list;
%type <node> constant expression assignable case;
%type <integer> next_instruction;
%type <stack> ablock argument_list non_empty_argument_list;
%type <stack> case_list;

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
        add_instruction(code_table, I_GOTO, NULL, NULL);
    } definition_list next_instruction {
        code_table->entries[0]->lhs = ir_node($3, NULL);
    } sblock 
    ;

definition_list: 
    /* Empty String */
    | definition definition_list
    ;

definition:
    check_type_literal identifier COLON constant check_arrow type_specifier COLON L_PARENTHESIS constant check_r_parenthesis {
        SYMTYPE* type = new_type(MT_ARRAY, $2);
        type->details.array->element_type = $6;
        if($4->meta != INT_CONSTANT) { 
            type_mismatch_error("constant integer", $4->type_name);
        } else {
            type->details.array->dimensions = $4->value.integer;
        }
        insert_new_symbol(type, $2, TYPE, "atype");
    }
    | check_type_literal identifier COLON constant check_arrow type_specifier {
        SYMTYPE* type = new_type(MT_ARRAY, $2);
        type->details.array->element_type = $6;
        if($4->meta != INT_CONSTANT) { 
            type_mismatch_error("constant integer", $4->type_name);
        } else {
            type->details.array->dimensions = $4->value.integer;
        }
        insert_new_symbol(type, $2, TYPE, "atype");
    }
    | check_type_literal identifier COLON pblock check_arrow type_specifier {
        SYMTYPE* type = new_type(MT_FUNCTION, $2);
        type->details.function->parameters = $4;
        type->details.function->return_type = new_symbol($6, $2, LOCAL, "local");
        insert_new_symbol(type, $2, TYPE, "ftype");
    }
    | FUNCTION identifier COLON type_specifier {
        insert_new_symbol($4, $2, FUNCTION, "function");
        function_context = stack_push(function_context, $4);
    } sblock { 
        SYMTAB* s = find_symbol($2); 
        NODE* n = add_instruction(code_table, I_LOOKUP, symbol_node(s), NULL);
        add_instruction(code_table, I_RETURN, n, NULL);
        function_context = stack_pop(function_context); 
    }
    | check_type_literal identifier COLON open_scope {
        new_type(MT_RECORD, $2);
    } dblock close_scope  {
        SYMTYPE* t = lookup_type($2);
        add_symbols_to_scope($4, $6);
        t->details.record->members = $4;
        insert_new_symbol(t, $2, TYPE, "rtype");
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
    check_l_bracket declaration_list check_r_bracket {
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
        symbol_context = stack_push(symbol_context, $1);
    } COLON identifier_list {
        $$ = $4;
        symbol_context = stack_pop(symbol_context);
    }
    ;


identifier_list:
    identifier assign_op constant COMMA identifier_list {
        SYMTYPE* type = stack_peek(symbol_context);
        $$ = add_symbols($5, new_symbol(type, $1, LOCAL, "local"));
    }
    | identifier assign_op constant {
        SYMTYPE* type = stack_peek(symbol_context);
        $$ = new_symbol(type, $1, LOCAL, "local");
        if(!type_check_binary_expression(I_ASSIGN, type->name, $3->type_name)) {

        }
    }
    | identifier COMMA identifier_list {
        SYMTYPE* type = stack_peek(symbol_context);
        $$ = add_symbols($3, new_symbol(type, $1, LOCAL, "local"));

    }
    | identifier {
        SYMTYPE* type = stack_peek(symbol_context);
        $$ = new_symbol(type, $1, LOCAL, "local");
    }
    ;

sblock:
    L_BRACE open_scope dblock {
        add_symbols_to_scope($2, $3);
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
        if(!compare_types("Boolean", $6->type_name)) {
            type_mismatch_error("Boolean", $6->type_name);
        }
        add_instruction(code_table, I_TEST_FALSE, $6, NULL);
    } next_instruction {
//    9                10
        add_instruction(code_table, I_GOTO, NULL, NULL);
    } SEMI_COLON next_instruction statement {
//    11         12               13        14
        NODE* n = ir_node($5, NULL);
        add_instruction(code_table, I_GOTO, n, NULL);
    } R_PARENTHESIS next_instruction {
//    15            16               17
        code_table->entries[$9]->lhs = ir_node($16, NULL);
    } sblock {
//    18     19
        NODE* n = ir_node($12, NULL);
        add_instruction(code_table, I_GOTO, n, NULL);
    } next_instruction {
//    20               21
        code_table->entries[$7]->rhs = ir_node($20, NULL);
    }

    | SWITCH L_PARENTHESIS expression check_r_parenthesis {
//    1      2             3          4                   5
        if(!compare_types("integer", $3->type_name)) {
            type_mismatch_error("integer", $3->type_name);
        }
        case_context = stack_push(case_context, $3);
    } case_list OTHERWISE COLON next_instruction sblock next_instruction {
//    6         7         8     9                 10     11
        STACK* s = $6;
        NODE* jmp = ir_node($11, NULL);
        while(s) {
            NODE* n = stack_peek(s);
            code_table->entries[n->value.instruction]->lhs = jmp;
            s = stack_pop(s);
        }
        case_context = stack_pop(case_context);
    }
    
    | IF L_PARENTHESIS expression next_instruction { 
//    1  2             3          4                5
        if(!compare_types("Boolean", $3->type_name)) {
            type_mismatch_error("Boolean", $3->type_name);
        }
        add_instruction(code_table, I_TEST_FALSE, $3, NULL);
    } check_r_parenthesis THEN sblock next_instruction {
//    6                   7    8      9                10
        add_instruction(code_table, I_GOTO, NULL, NULL);
    } next_instruction {
//    11               12
        code_table->entries[$4]->rhs = ir_node($11, NULL);
    } ELSE sblock next_instruction {
//    13   14     15               16
        code_table->entries[$9]->lhs = ir_node($15, NULL);
    }

    | WHILE L_PARENTHESIS next_instruction expression next_instruction {
//    1     2             3                4          5                6
        if(!compare_types("Boolean", $4->type_name)) {
            type_mismatch_error("Boolean", $4->type_name);
        }
        add_instruction(code_table, I_TEST_FALSE, $4, NULL);
    } check_r_parenthesis sblock {
//    7                   8      9
        add_instruction(code_table, I_GOTO, $4, NULL);
    } next_instruction {
//    10               11
        code_table->entries[$5]->rhs = ir_node($10, NULL);
    }

    | assignable assign_op expression semi_colon_after_statement {
        if(!compare_types($1->type_name, $3->type_name)) {
            SYMTYPE* st = stack_peek(function_context);
            if(st && compare_types(st->name, $1->type_name)) {
                NODE* n = add_instruction(code_table, I_LOOKUP, $1, NULL);
                add_instruction(code_table, I_ASSIGN, n, $3);
            } else {
                type_mismatch_error($1->type_name, $3->type_name);
            }
        } else {
            NODE* n = add_instruction(code_table, I_LOOKUP, $1, NULL);
            add_instruction(code_table, I_ASSIGN, n, $3);
        }
    }

    | mem_op assignable semi_colon_after_statement {
        add_instruction(code_table, $1, $2, NULL);
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
        NODE* cmp = stack_peek(case_context);

        NODE* n = add_instruction(code_table, I_EQUAL, $2, cmp); 
        add_instruction(code_table, I_TEST_FALSE, n, NULL);
    } COLON sblock next_instruction {
//    5     6      7
        if(!compare_types("integer", $2->type_name)) {
            type_mismatch_error("integer", $2->type_name);
        } else
        $$ = add_instruction(code_table, I_GOTO, NULL, NULL);
        code_table->entries[$3 + 1]->rhs = ir_node($7 + 1, NULL);
    }
    ;

assignable:
    identifier { 
        SYMTAB* s = find_symbol($1); 

        if (!s) {
            SYMTYPE* st = stack_peek(function_context);
            if(st) {
                s = find_entry(st->details.function->parameters->symbols, $1);
            }

            if(!s) {
                symbol_not_found_error($1, "variable");
            }

            $$ = symbol_node(s);
        } else if (s->meta == TYPE) {
            type_as_var_error(s->name);
            $$ = symbol_node(NULL);
        } else {
            $$ = symbol_node(s);
        }
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
                    $$ = symbol_node(NULL);
                } else {
                    $$ = symbol_node(s);
                }
            } 
        } else {
            $$ = symbol_node(NULL);
        }
    }
    | assignable ablock {
        if($1 && $1->value.symbol) {
            if(check_metatype($1->value.symbol->type, MT_FUNCTION)) {
                STACK* args = $2;
                SYMTAB* params = $1->value.symbol->type->details.function->parameters->symbols;
                int expected = 0;
                int actual = 0;
                while(params) {
                    if(!args) {
                        while(params) { params = params->next; expected++; }
                        argument_count_mismatch(expected, actual);
                        break;
                    }
                    actual++;
                    NODE* n = stack_peek(args);
                    
                    if(!compare_types(params->type->name, n->type_name)) {
                        type_mismatch_error(params->type->name, n->type_name);
                    } else {
                        add_instruction(code_table, I_PARAM, n, NULL);
                    }

                    args = stack_pop(args);
                    params = params->next;
                    expected++;
                }

                if(args) {
                    while(args) { args = stack_pop(args); actual++; }
                    argument_count_mismatch(expected, actual);
                }

                $$ = add_instruction(code_table, I_CALL, $1, int_node(expected));
            } else if(check_metatype($1->value.symbol->type, MT_ARRAY)) {
                /* 
                    TODO: ARRAY ACCESS
                */
                $$ = add_instruction(code_table, I_ARRAY, $1, NULL);
                // printf("Array access: %s", $1->name);
            } else {
                incorrect_type_error($1->value.symbol->name, "array or function");
            }
        } else {

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

expression:
    expression binary_operator expression {
        /*
            TODO: 
                Boolean shortcircuiting

        */
        TC_RESULT r = type_check_binary_expression($2, $1->type_name, $3->type_name);
        switch(r) {
            case FAIL:
                invalid_binary_expression($2, $1->type_name, $3->type_name);
            case PASS: 
                $$ = add_instruction(code_table, $2, $1, $3);
                break;
            case COERCE_RHS: {
                NODE* n = add_instruction(code_table, I_INT2REAL, $3, NULL);
                $$ = add_instruction(code_table, $2, $1, n);
                break;
            }
            case COERCE_LHS: {
                NODE* n = add_instruction(code_table, I_INT2REAL, $1, NULL);
                $$ = add_instruction(code_table, $2, n, $3);
                break;
            }
        }
    }
    | expression post_unary_operator {
        if(!type_check_unary_expression($2, $1->type_name)) {
            invalid_unary_expression($2, $1->type_name);
        }

        $$ = add_instruction(code_table, $2, $1, NULL);
    }
    | pre_unary_operator expression %prec pre_unary_prec {
        if(!type_check_unary_expression($1, $2->type_name)) {
            invalid_unary_expression($1, $2->type_name);    
        }
        $$ = add_instruction(code_table, $1, $2, NULL);
    }
    | L_PARENTHESIS expression R_PARENTHESIS {
        $$ = $2;
    }
    | constant 
    | assignable { 
        if($1->meta == SYMBOL) {
            $$ = add_instruction(code_table, I_LOOKUP, $1, NULL);
        }
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
SYMTYPE** get_types() {
    return &types;
}

SCOPE** get_symbol_table() {
    return &symbols;
}

IRTABLE** get_intermediate_code() {
    return &code_table;
}

STACK** get_errors() {
    return &errors;
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

void notify_error(char* e) {
    yyerror(e);
    errors = stack_push(errors, strdup(e));
}

void argument_count_mismatch(int expected, int actual) {
    const char format[] = "LINE %d:%d - ERROR: Incorrect argument count (Expected: %d, Actual: %d)\n";
    
    char dest[strlen(format) + 26];

    sprintf(dest, format, get_row(), get_column(), expected, actual);
    notify_error(dest);
    yyerrok;
}

void invalid_unary_expression(int op, char* t1) {
    const char format[] = "LINE %d:%d - ERROR: unary operator '%s' cannot be applied to type '%s'\n";
    
    char dest[strlen(format) + (t1 ? strlen(t1) : 0) + 26];

    sprintf(dest, format, get_row(), get_column(), get_op_string(op) , t1);
    notify_error(dest);
    yyerrok;
}

void invalid_binary_expression(int op, char* t1, char* t2) {
    const char format[] = "LINE %d:%d - ERROR: binary operator '%s' cannot be applied to type '%s' and '%s'\n";
    
    char dest[strlen(format) + (t1 ? strlen(t1) : 0) + (t2 ? strlen(t2) : 0) + 26];

    sprintf(dest, format, get_row(), get_column(), get_op_string(op), t2, t1);
    notify_error(dest);
    yyerrok;
}

void type_mismatch_error(char* t1, char* t2) {
    const char format[] = "LINE %d:%d - ERROR: type '%s' is incompatible with expected type '%s'\n";
    
    char dest[strlen(format) + (t1 ? strlen(t1) : 0) + (t2 ? strlen(t2) : 0) + 21];

    sprintf(dest, format, get_row(), get_column(), t2, t1);
    notify_error(dest);
    yyerrok;
}

void type_as_var_error(char* name) {
    const char format[] = "LINE %d:%d - ERROR: %s, a type, is used here as a variable\n";
    
    char dest[strlen(format) + strlen(name) + 21];

    sprintf(dest, format, get_row(), get_column(), name);
    notify_error(dest);
    yyerrok;
}

void incorrect_type_error(char* name, char* type) {
    const char format[] = "LINE %d:%d - ERROR: %s is not of type %s\n";
    
    char dest[strlen(format) + strlen(name) + strlen(type)  + 21];

    sprintf(dest, format, get_row(), get_column(), name, type);
    notify_error(dest);
    yyerrok;
}

void symbol_not_found_error(char* name, char* style) {
    const char format[] = "LINE %d:%d - ERROR: %s, used here as a %s, has not been declared at this point in the program.\n";

    char dest[strlen(format) + strlen(name) + strlen(style)  + 21];

    sprintf(dest, format, get_row(), get_column(), name, style);
    
    notify_error(dest);
    yyerrok;
}

void syntax_error(char* expected) {
    const char format[] = "LINE %d:%d - ERROR: %s\n";
    
    char dest[strlen(format) + strlen(expected) + 21];

    sprintf(dest, format, get_row(), get_column(), expected);
    notify_error(dest);
}

void yyerror (char *s) {

 }
