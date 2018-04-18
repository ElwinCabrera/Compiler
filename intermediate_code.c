#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "intermediate_code.h"
#include "address.h"


static INTERMEDIATE_CODE* intermediate_code;

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

TAC* new_tac(TAC_OP op, ADDRESS* x, ADDRESS* y, ADDRESS* r) {
    TAC* code = malloc(sizeof(TAC));
    code->op = op;
    code->x = x;
    code->y = y;
    code->result = r;
    return code;
}

ADDRESS* add_code(INTERMEDIATE_CODE* table, TAC* code) {
    if(!table || !code) {
        return NULL;
    }
    code->label = table->next_instruction;
    table->entries[table->next_instruction++] = code;
    return code->result;
}

void print_intermediate_code(INTERMEDIATE_CODE* table, FILE* f) {
    if(!table) {
        return;
    }

    for(int i = 0; i < table->next_instruction; i++) {
        print_tac(table->entries[i], f);
    }
}

void print_tac(TAC* code, FILE* f) {
    
    if(!code) {
        return;
    }

    char* str = create_tac_string(code);

    if(f) {
        fprintf(f, str);
    } else {
        printf(str);
    }
}

const char* get_op_string(TAC_OP op) {
    switch(op) {
        case I_ASSIGN:
            return "assign";
        case I_LOOKUP:
            return "lookup";
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
            return "test";
        case I_TEST_FALSE:
            return "testfalse";
        case I_GOTO:
            return "goto";
        case I_RESERVE:
            return "reserve";
        case I_RELEASE:
            return "release";
        case I_ARRAY:
            return "array";
        default:
            return NULL;
    }
}

int get_str_size(char* x, char* y, char* result) {
    int x_len = x ? strlen(x) : 0;
    int y_len = y ? strlen(y) : 0;
    int result_len = result ? strlen(result) : 0;
    return x_len + y_len + result_len + 1;
}

char* create_tac_string(TAC* code) {
    char* x = code->x ? create_address_string(code->x) : NULL;
    char* y = code->y ? create_address_string(code->y) : NULL;
    char* result = code->result ? create_address_string(code->result) : NULL;
    int size = get_str_size(x, y, result);
    char* str = malloc(size * sizeof(char) + 20);
    switch(code->op) {
        case I_ASSIGN:
            sprintf(str, "[%02d]\t %s := %s\n", code->label, x, y);
            break;
        case I_LOOKUP:
            sprintf(str, "[%02d]\t %s = ID %s\n", code->label, result, x);
            break;
        case I_ADD:
            sprintf(str, "[%02d]\t %s = %s + %s\n", code->label, result, x, y);
            break;
        case I_SUB:
            if(code->y) {
                sprintf(str, "[%02d]\t %s = %s - %s\n", code->label, result, x, y);
            } else {
                sprintf(str, "[%02d]\t %s = - %s\n", code->label, result, x);
            }
            break;
        case I_MULTIPLY:
            sprintf(str, "[%02d]\t %s = %s * %s\n", code->label, result, x, y);
            break;
        case I_DIVIDE:
            sprintf(str, "[%02d]\t %s = %s / %s\n", code->label, result, x, y);
            break;
        case I_MODULUS: 
            sprintf(str, "[%02d]\t %s = %s MOD %s\n", code->label, result, x, y);
            break;
        case I_LESS_THAN:
            sprintf(str, "[%02d]\t %s = %s < %s\n", code->label, result, x, y);
            break;
        case I_EQUAL:
            sprintf(str, "[%02d]\t %s = %s == %s\n", code->label, result, x, y);
            break;
        case I_REAL2INT:
            sprintf(str, "[%02d]\t %s = r2i %s\n", code->label, result, x);
            break;
        case I_INT2REAL: 
            sprintf(str, "[%02d]\t %s = i2r %s\n", code->label, result, x);
            break;
        case I_IS_NULL:
            sprintf(str, "[%02d]\t %s = isNull %s\n", code->label, result, x);
            break;
        case I_NOT:
            sprintf(str, "[%02d]\t %s = not %s\n", code->label, result, x);
            break;
        case I_AND:
            sprintf(str, "[%02d]\t %s = %s AND %s\n", code->label, result, x, y);
            break;
        case I_OR:
            sprintf(str, "[%02d]\t %s = %s OR %s\n", code->label, result, x, y);
            break;
        case I_PARAM:
            sprintf(str, "[%02d]\t param %s\n", code->label, x);
            break;
        case I_CALL:
            sprintf(str, "[%02d]\t call %s %s\n", code->label, x, y);
            break;
        case I_RETURN:
            sprintf(str, "[%02d]\t return %s\n", code->label, result);
            break;
        case I_TEST:
            sprintf(str, "[%02d]\t if %s goto %s\n", code->label, x, result);
            break;
        case I_TEST_FALSE:
            sprintf(str, "[%02d]\t ifFalse %s goto %s\n", code->label, x, result);
            break;
        case I_TEST_NOTEQUAL:
            sprintf(str, "[%02d]\t if %s != %s goto %s\n", code->label, x, y, result);
            break;
        case I_GOTO:
            sprintf(str, "[%02d]\t goto %s\n", code->label, result);
            break;
        case I_RESERVE:
            sprintf(str, "[%02d]\t reserve %s\n", code->label, x);
            break;
        case I_RELEASE:
            sprintf(str, "[%02d]\t release %s\n", code->label, x);
            break;
        case I_ARRAY:
            sprintf(str, "[%02d]\t %s = %s[%s]\n", code->label, result, x, y);
            break;
        default:
            sprintf(str, "UNKNOWN OP: %d\n", code->op);
    }

    if(x) free(x);
    if(y) free(y);
    if(result) free(result);

    return str;
}
