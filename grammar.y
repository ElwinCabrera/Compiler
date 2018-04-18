%{
#include <stdio.h>
#include <string.h>
#include "stack.h"
#include "symbol_table.h"
#include "types.h"
#include "address.h"
#include "intermediate_code.h"

static int scope_counter = 0;
static SCOPE* symbols;
static STACK* errors;
static int yyerrstatus;
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
SYMTAB* insert_new_symbol(SYMTYPE*, char*, int, char*);


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
    struct stack* stack;
    struct address* address;
    struct expression* exp;
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
%type <symbol> assignable;
%type <address> case constant expression;
%type <integer> next_instruction;
%type <stack> ablock argument_list non_empty_argument_list case_list;

// Operator precedence conflicts, but the generated state machine
// chooses the correct state, we just need to handle precedence
%expect 20

%left '*' '/' '%'
%left '+' '-'
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
    } definition_list next_instruction sblock {
        code_table->entries[0]->result = label_address($3);
    }
    ;

definition_list: 
    /* Empty String */
    | definition definition_list
    ;

definition:
    check_type_literal identifier COLON constant check_arrow type_specifier COLON L_PARENTHESIS constant check_r_parenthesis {
        SYMTYPE* type = new_type(MT_ARRAY, $2);

        if(type) {
            type->element_type = $6;

            if($4->meta != INT_CONSTANT) { 
                type_mismatch_error("constant integer", $4->type ? $4->type->name : "NULL");
            } else {
                type->dimensions = $4->value.integer;
            }
        }

        insert_new_symbol(type, $2, TYPE, "atype");
    }
    | check_type_literal identifier COLON constant check_arrow type_specifier {
        SYMTYPE* type = new_type(MT_ARRAY, $2);
        if(type) {
            type->element_type = $6;
            if($4->meta != INT_CONSTANT) { 
                type_mismatch_error("constant integer", $4->type ? $4->type->name : "NULL");
            } else {
                type->dimensions = $4->value.integer;
            }
        }

        insert_new_symbol(type, $2, TYPE, "atype");
    }
    | check_type_literal identifier COLON pblock check_arrow type_specifier {
        SYMTYPE* type = new_type(MT_FUNCTION, $2);
        if(type) {
            type->parameters = $4;
            type->ret = new_symbol($6, $2, LOCAL, "local");
        }
        insert_new_symbol(type, $2, TYPE, "ftype");
    }
    | FUNCTION identifier COLON type_specifier {
        SYMTAB* s = insert_new_symbol($4, $2, FUNCTION, "function");
        function_context = stack_push(function_context, s);
    } sblock { 
        ADDRESS* a = symbol_address($4->ret);
        add_code(code_table, new_tac(I_RETURN, NULL, NULL, a));
        function_context = stack_pop(function_context); 
    }
    | check_type_literal identifier COLON open_scope {
        new_type(MT_RECORD, $2);
    } dblock close_scope  {
        SYMTYPE* t = lookup_type($2);
        add_symbols_to_scope($4, $6);
        t->members = $4;
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
        SYMTAB* s = new_symbol(type, $1, LOCAL, "local");
        if(!s || !type || type != $3->type) {
            type_mismatch_error(type ? type->name : "NULL", 
                $3->type ? $3->type->name : "NULL");
        } else {
            add_code(code_table, new_tac(I_ASSIGN, symbol_address(s), $3, NULL));
        }
        $$ = add_symbols($5, s);
    }
    | identifier assign_op constant {
        SYMTYPE* type = stack_peek(symbol_context);
        SYMTAB* s = new_symbol(type, $1, LOCAL, "local");
        if(!s || !type || type != $3->type) {
            type_mismatch_error(type ? type->name : "NULL", 
                $3->type ? $3->type->name : "NULL");
        } else {
            add_code(code_table, new_tac(I_ASSIGN, symbol_address(s), $3, NULL));
        }
        $$ = new_symbol(type, $1, LOCAL, "local");
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
        if(!check_typename($6->type, "Boolean")) {
            type_mismatch_error("Boolean", $6->type ? $6->type->name : "NULL");
        }
        add_code(code_table, new_tac(I_TEST_FALSE, $6, NULL, NULL));
    } next_instruction {
//    9                10
        add_code(code_table, new_tac(I_GOTO, NULL, NULL, NULL));
    } SEMI_COLON next_instruction statement {
//    11         12               13        14
        add_code(code_table, new_tac(I_GOTO, NULL, NULL, label_address($5)));
    } R_PARENTHESIS next_instruction sblock {
//    15            16               17     18
        code_table->entries[$9]->result = label_address($16);
        add_code(code_table, new_tac(I_GOTO, NULL, NULL, label_address($12)));
    } next_instruction {
//    19               20
        code_table->entries[$7]->result = label_address($19);
    }

    | SWITCH L_PARENTHESIS expression check_r_parenthesis {
//    1      2             3          4                   5
        if(!check_typename($3->type, "integer")) {
            type_mismatch_error("integer", $3->type ? $3->type->name : "NULL");
        }
        case_context = stack_push(case_context, $3);
    } case_list OTHERWISE COLON next_instruction sblock next_instruction {
//    6         7         8     9                 10     11
        STACK* s = $6;
        ADDRESS* jmp = label_address($11);
        while(s) {
            ADDRESS* a = stack_peek(s);
            code_table->entries[a->value.label]->result = jmp;
            s = stack_pop(s);
        }
        case_context = stack_pop(case_context);
    }
    
    | IF L_PARENTHESIS expression next_instruction { 
//    1  2             3          4                5
        if(!check_typename($3->type, "Boolean")) {
            type_mismatch_error("Boolean", $3->type ? $3->type->name : "NULL");
        }
        add_code(code_table, new_tac(I_TEST_FALSE, $3, NULL, NULL));
    } check_r_parenthesis THEN sblock next_instruction {
//    6                   7    8      9                10
        add_code(code_table, new_tac(I_GOTO, NULL, NULL, NULL));
    } next_instruction ELSE sblock next_instruction {
//    11               12   13     14
        code_table->entries[$4]->result = label_address($11);
        code_table->entries[$9]->result = label_address($14);
    }

    | WHILE L_PARENTHESIS next_instruction expression next_instruction {
//    1     2             3                4          5                6
        if(!check_typename($4->type, "Boolean")) {
            type_mismatch_error("Boolean", $4->type ? $4->type->name : "NULL");
        }
        add_code(code_table, new_tac(I_TEST_FALSE, $4, NULL, NULL));
    } check_r_parenthesis sblock {
//    7                   8      9
        add_code(code_table, new_tac(I_GOTO, NULL, NULL, label_address($3)));
    } next_instruction {
//    10               11
        code_table->entries[$5]->result = label_address($10);
    }

    | assignable assign_op expression semi_colon_after_statement {

        SYMTAB* s = $1;

        if(s && check_metatype(s->type, MT_FUNCTION)) {
            s = s->type->ret;
        }

        if(!s || !s->type || s->type != $3->type) {
            type_mismatch_error($1 ? $1->type ? $1->type->name : "NULL" : "NULL", 
                $3->type ? $3->type->name : "NULL");
        } else {
            add_code(code_table, new_tac(I_ASSIGN, symbol_address(s), $3, NULL));
        }
    }

    | mem_op assignable semi_colon_after_statement {
        add_code(code_table, new_tac($1, symbol_address($2), NULL, NULL));
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
        ADDRESS* exp = stack_peek(case_context);
        add_code(code_table, new_tac(I_TEST_NOTEQUAL, exp, $2, NULL));
    } COLON sblock next_instruction {
//    5     6      7
        if(!check_typename($2->type, "integer")) {
            type_mismatch_error("integer", $2->type ? $2->type->name : "NULL");
        } else {
            $$ = label_address($7);
            add_code(code_table, new_tac(I_GOTO, NULL, NULL, NULL));
            code_table->entries[$3]->result = label_address($7 + 1);
        }
    }
    ;

assignable:
    identifier { 
        SYMTAB* s = NULL;
        SYMTAB* fn = stack_peek(function_context);

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
            $$ = NULL;
        } else if (s->meta == TYPE) {
            type_as_var_error(s->name);
            $$ = s;
        } else {
            $$ = s;
        }
    }
    | assignable rec_op identifier {
        if($1) {
            if(!check_metatype($1->type, MT_RECORD)) {
                incorrect_type_error($1->name, "record");
                $$ = NULL;
            } else {
                SYMTAB* s = find_entry($1->type->members->symbols, $3); 
                if(!s) {
                    symbol_not_found_error($3, "record member");
                    $$ = NULL;
                } else {
                    $$ = s;
                }
            } 
        } else {
            $$ = NULL;
        }
    }
    | assignable ablock {
        if(check_metatype($1->type, MT_FUNCTION)) {
            STACK* args = $2;
            SYMTAB* params = $1->type->parameters->symbols;

            int expected = 0;
            int actual = 0;
            while(params) {
                if(!args) {
                    while(params) { params = params->next; expected++; }
                    argument_count_mismatch(expected, actual);
                    break;
                }
                actual++;
                ADDRESS* a = stack_peek(args);
                
                if(params->type && params->type == a->type) {
                    add_code(code_table, new_tac(I_PARAM, a, NULL, NULL));
                } else {
                    type_mismatch_error(params->type ? params->type->name : "NULL", 
                        a->type ? a->type->name : "NULL");
                }

                args = stack_pop(args);
                params = params->next;
                expected++;
            }

            if(args) {
                while(args) { args = stack_pop(args); actual++; }
                argument_count_mismatch(expected, actual);
            }
            
            add_code(code_table, new_tac(I_CALL, symbol_address($1), int_address(expected), NULL));
            $$ = $1->type->ret;
        } else if(check_metatype($1->type, MT_ARRAY)) {
            
            /* 
                TODO: ARRAY ACCESS
            */
            // $$ = add_instruction(code_table, I_ARRAY, $1, NULL);
            // printf("Array access: %s", $1->name);
        } else {
            incorrect_type_error($1->name, "array or function");
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
        TC_RESULT r = type_check_binary_expression($2, $1->type, $3->type);
        switch(r) {
            case FAIL:
                invalid_binary_expression($2, $1->type ? $1->type->name : "NULL",
                    $3->type ? $3->type->name : "NULL");
            case PASS: {
                $$ = add_code(code_table, new_tac($2, $1, $3, temp_address(lval_type($2, $1->type, $3->type))));
                break;
            }
            case COERCE_RHS: {
                ADDRESS* a = add_code(code_table, new_tac(I_INT2REAL, $3, NULL, temp_address($1->type)));
                $$ = add_code(code_table, new_tac($2, $1, a, temp_address($1->type)));
                break;
            }
            case COERCE_LHS: {
                ADDRESS* a = add_code(code_table, new_tac(I_INT2REAL, $1, NULL, temp_address($3->type)));
                $$ = add_code(code_table, new_tac($2, a, $3, temp_address($3->type)));
                break;
            }
        }
    }
    | expression post_unary_operator {
        if(!type_check_unary_expression($2, $1->type)) {
            invalid_unary_expression($2, $1->type ? $1->type->name : "NULL");
        }
        ADDRESS* a = temp_address(lval_type($2, $1->type, NULL));
        $$ = add_code(code_table, new_tac($2, $1, NULL, a));
    }
    | pre_unary_operator expression %prec pre_unary_prec {
        if(!type_check_unary_expression($1, $2->type)) {
            invalid_unary_expression($1, $2->type ? $2->type->name : "NULL");
        }
        ADDRESS* a = temp_address(lval_type($1, $2->type, NULL));
        $$ = add_code(code_table, new_tac($1, $2, NULL, a));
    }
    | L_PARENTHESIS expression R_PARENTHESIS {
        $$ = $2;
    }
    | constant 
    | assignable { 
        $$ = symbol_address($1);
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

SCOPE** get_symbol_table() {
    return &symbols;
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

SYMTAB* insert_new_symbol(SYMTYPE* type, char* name, int meta, char* extra) {
    SYMTAB* s = new_symbol(type, name, meta, extra);
    add_symbols_to_scope(symbols, s);
    return s;
}

void initialize_structs() {
    open_scope();
    code_table = get_intermediate_code();
    types = get_type_container();

    new_type(MT_PRIMITIVE, "string");
    new_type(MT_PRIMITIVE, "real");
    new_type(MT_PRIMITIVE, "Boolean");
    new_type(MT_PRIMITIVE, "integer");
    new_type(MT_PRIMITIVE, "character");
    new_type(MT_PRIMITIVE, "nullconst");
}

SYMTYPE* new_type(int type, char* name) {
    return add_type(types, type, name);
}

SYMTYPE* lookup_type(char* name) {
    return find_type(types, name);
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
