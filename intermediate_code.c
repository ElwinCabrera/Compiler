#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "intermediate_code.h"
#include "address.h"


static INTERMEDIATE_CODE* intermediate_code;

/*
    Creates (if necessary) and returns a pointer to an intermediate code
    representation
*/
INTERMEDIATE_CODE* get_intermediate_code() {
    if(!intermediate_code) {
        // Allocate room for 1000 three-address codes
        intermediate_code = malloc(sizeof(INTERMEDIATE_CODE));
        intermediate_code->capacity = 1000;
        intermediate_code->entries = malloc(sizeof(TAC) * 1000);
        intermediate_code->next_instruction = 0;
    }
    return intermediate_code;
}

/*
    Creates a new three address code
*/
TAC* new_tac(TAC_OP op, ADDRESS* x, ADDRESS* y, ADDRESS* r) {
    TAC* code = malloc(sizeof(TAC));
    code->result_live = false;
    code->x_live = false;
    code->y_live = false;
    code->result_next = 0;
    code->x_next = 0;
    code->y_next = 0;
    code->op = op;
    code->x = x;
    code->y = y;
    code->result = r;
    code->leader = false;
    return code;
}

/*
    Adds a three address code to the table if both are not null
    This labels the TAC with the next open label.
*/
ADDRESS* add_code(INTERMEDIATE_CODE* table, TAC* code) {
    if(!table || !code) {
        return NULL;
    }
    code->label = table->next_instruction;
    table->entries[table->next_instruction++] = code;
    return code->result;
}

void backpatch(INTERMEDIATE_CODE* table, int from, int to) {
    if(!table || table->next_instruction <= from) {
        return;
    }

    table->entries[from]->result = label_address(to);
}

/*
    Helper function to print a full intermediate representation
*/
void print_intermediate_code(INTERMEDIATE_CODE* table, FILE* f) {
    if(!table) {
        return;
    }

    for(int i = 0; i < table->next_instruction; i++) {
        print_tac(table->entries[i], f);
    }
}

/*
    Print a single TAC
*/
void print_tac(TAC* code, FILE* f) {
    
    if(!code) {
        return;
    }

    if(!f) {
        f = stdout;
    }

    char* x = code->x ? create_address_string(code->x) : NULL;
    char* y = code->y ? create_address_string(code->y) : NULL;
    char* result = code->result ? create_address_string(code->result) : NULL;

    switch(code->op) {
        case I_ASSIGN:
            fprintf(f, "[%03d]\t %s = %s\n", code->label, x, y);
            break;
        case I_RECORD_ACCESS:
            fprintf(f, "[%03d]\t %s = %s.%s\n", code->label, result, x, y);
            break;
        case I_RECORD_ASSIGN:
            fprintf(f, "[%03d]\t %s.%s = %s\n", code->label, result, x, y);
            break;
        case I_ADD:
            fprintf(f, "[%03d]\t %s = %s + %s\n", code->label, result, x, y);
            break;
        case I_SUB:
            if(code->y) {
               fprintf(f, "[%03d]\t %s = %s - %s\n", code->label, result, x, y);
            } else {
                fprintf(f, "[%03d]\t %s = - %s\n", code->label, result, x);
            }
            break;
        case I_MULTIPLY:
            fprintf(f, "[%03d]\t %s = %s * %s\n", code->label, result, x, y);
            break;
        case I_DIVIDE:
            fprintf(f, "[%03d]\t %s = %s / %s\n", code->label, result, x, y);
            break;
        case I_MODULUS: 
            fprintf(f, "[%03d]\t %s = %s MOD %s\n", code->label, result, x, y);
            break;
        case I_LESS_THAN:
            fprintf(f, "[%03d]\t %s = %s < %s\n", code->label, result, x, y);
            break;
        case I_EQUAL:
            fprintf(f, "[%03d]\t %s = %s == %s\n", code->label, result, x, y);
            break;
        case I_REAL2INT:
            fprintf(f, "[%03d]\t %s = r2i %s\n", code->label, result, x);
            break;
        case I_INT2REAL: 
            fprintf(f, "[%03d]\t %s = i2r %s\n", code->label, result, x);
            break;
        case I_IS_NULL:
            fprintf(f, "[%03d]\t %s = %s == null\n", code->label, result, x);
            break;
        case I_NOT:
            fprintf(f, "[%03d]\t %s = not %s\n", code->label, result, x);
            break;
        case I_AND:
            fprintf(f, "[%03d]\t %s = %s AND %s\n", code->label, result, x, y);
            break;
        case I_OR:
            fprintf(f, "[%03d]\t %s = %s OR %s\n", code->label, result, x, y);
            break;
        case I_PARAM:
            fprintf(f, "[%03d]\t param %s\n", code->label, x);
            break;
        case I_CALL:
            fprintf(f, "[%03d]\t %s = call %s %s\n", code->label, result, x, y);
            break;
        case I_RETURN:
            fprintf(f, "[%03d]\t return %s\n", code->label, result);
            break;
        case I_TEST:
            fprintf(f, "[%03d]\t if %s goto %s\n", code->label, x, result);
            break;
        case I_TEST_FALSE:
            fprintf(f, "[%03d]\t ifFalse %s goto %s\n", code->label, x, result);
            break;
        case I_TEST_NOTEQUAL:
            fprintf(f, "[%03d]\t if %s != %s goto %s\n", code->label, x, y, result);
            break;
        case I_GOTO:
            fprintf(f, "[%03d]\t goto %s\n", code->label, result);
            break;
        case I_RESERVE:
            fprintf(f, "[%03d]\t %s = malloc(%s)\n", code->label, result, x);
            break;
        case I_RELEASE:
            fprintf(f, "[%03d]\t release(%s,%s)\n", code->label, result, x);
            break;
        case I_ARRAY_ASSIGN:
            fprintf(f, "[%03d]\t %s[%s] = %s\n", code->label, result, x, y);
            break;
        case I_ARRAY_ACCESS:
            fprintf(f, "[%03d]\t %s = %s[%s]\n", code->label, result, x, y);
            break;
        case I_NOP:
            fprintf(f, "[%03d]\t NOP\n", code->label);
            break;
        default:
            fprintf(f, "UNKNOWN OP: %d\n", code->op);
    }

    if(x) free(x);
    if(y) free(y);
    if(result) free(result);
}

/*
    String representation of an operator
*/
const char* get_op_string(TAC_OP op) {
    switch(op) {
        case I_ASSIGN:
            return "=";
        case I_RECORD_ACCESS:
            return "x = y.z";
        case I_RECORD_ASSIGN:
            return "x.y = z";
        case I_ADD:
            return "+";
        case I_SUB:
            return "-";
        case I_MULTIPLY:
            return "*";
        case I_DIVIDE:
            return "/";
        case I_MODULUS:
            return "%";
        case I_LESS_THAN:
            return ">";
        case I_EQUAL:
            return "==";
        case I_REAL2INT:
            return "r2i";
        case I_INT2REAL:
            return "i2r";
        case I_IS_NULL:
            return "isNull";
        case I_NOT:
            return "!";
        case I_AND:
            return "&";
        case I_OR:
            return "|";
        case I_PARAM:
            return "param";
        case I_CALL:
            return "call";
        case I_RETURN:
            return "return";
        case I_TEST:
            return "if x goto L";
        case I_TEST_FALSE:
            return "ifFalse x goto L";
        case I_TEST_NOTEQUAL:
            return "if x != y goto L";
        case I_GOTO:
            return "goto";
        case I_RESERVE:
            return "reserve";
        case I_RELEASE:
            return "release";
        case I_ARRAY_ACCESS:
            return "=[]";
        case I_ARRAY_ASSIGN:
            return "[]=";
        default:
            return NULL;
    }
}

bool code_is_jump(TAC* code) {
    if(!code) {
        return false;
    }
    TAC_OP op = code->op;
    return op == I_GOTO || op == I_RETURN || op == I_CALL ||
        op == I_TEST_FALSE || op == I_TEST || op == I_TEST_NOTEQUAL;
}