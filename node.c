#include <string.h>
#include "node.h"
#include "types.h"
#include "symbolTable.h"

static int node_count = 0;

NODE* new_node() {
    NODE* n = malloc(sizeof(NODE));
    n->id = node_count++;
    return n;
}

void free_node(NODE* n) {
    free(n->type_name);
    free(n);
}

NODE* int_node(int value) {
    NODE* n = new_node();
    n->meta = INT_CONSTANT;
    n->type_name = strdup("integer");
    n->value.integer = value;
    return n;
}

NODE* boolean_node(int value) {
    NODE* n = new_node();
    n->meta = BOOLEAN_CONSTANT;
    n->type_name = strdup("Boolean");
    n->value.boolean = value;
    return n;
}

NODE* real_node(double value) {
    NODE* n = new_node();
    n->meta = REAL_CONSTANT;
    n->type_name = strdup("real");
    n->value.real = value;
    return n;
}

NODE* char_node(char value) {
    NODE* n = new_node();
    n->meta = CHAR_CONSTANT;
    n->type_name = strdup("character");
    n->value.character = value;
    return n;
}

NODE* string_node(char* value) {
    NODE* n = new_node();
    n->meta = STRING_CONSTANT;
    n->type_name = strdup("string");
    n->value.string = value;
    return n;
}

NODE* null_node() {
    NODE* n = new_node();
    n->meta = NULL_CONSTANT;
    return n;
}

NODE* symbol_node(struct symtab* value) {
    NODE* n = new_node();
    n->meta = SYMBOL;
    if(value && value->type) { 
        n->type_name = strdup(value->type->name);
    } else {
        n->type_name = NULL;
    }
    n->value.symbol = value;
    return n;
}

NODE* ir_node(struct ir* i, char* type_name) {
    NODE* n = new_node();
    n->meta = CODE;
    if(type_name) {
        n->type_name = strdup(type_name);
    } else {
        n->type_name = NULL;
    }
    n->value.instruction = i;
    return n;
}