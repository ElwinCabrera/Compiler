#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol_table.h"
#include "asc.h"

#define MAX_LINE 1024

static ASC* asc_internal = NULL;

void asc_init(FILE* f) {
    if(asc_internal) {
        //free_asc(asc_internal);
    }

    asc_internal = malloc(sizeof(ASC));
    asc_internal->file = f;
    asc_internal->lines = NULL;
    asc_new_line(1);
}

void asc_error(char* e) {
    asc_new_line(-1);
    asc_append(e);
}

void asc_line_scope(SCOPE* s) {
    if(!asc_internal) {
        return;
    }

    LINE* l = asc_internal->lines->value;
    l->scope = s;
}

void asc_new_line(int n) {

    if(!asc_internal) {
        return;
    }

    LINE* l = malloc(sizeof(LINE));
    l->lineno = n;
    l->text = malloc(MAX_LINE * sizeof(char));
    memset(l->text, 0, MAX_LINE);

    asc_internal->lines = ll_insertfront(asc_internal->lines, l);
}

void asc_print(FILE* f, LINE* l) {
    if(!f || !l) {
        return;
    }

    if(l->lineno >= 0) {
        fprintf(f, "%d: \t\t %s", l->lineno, l->text);
    } else {
        fprintf(f, "%s", l->text);
    }
}

void asc_append(char* str) {

    if(!asc_internal) {
        return;
    }

    LINE* l = ll_value(asc_internal->lines);
    if(!l) {
        printf("FATAL ERROR: Unable to append to line in ASC");
        return;
    }

    strcat(l->text, str);
}


void asc_print_file() {
    if(!asc_internal) {
        return;
    }

    ll_reverse(&asc_internal->lines);

    LINKED_LIST* lines = asc_internal->lines;

    while(lines) {
        LINE* l = ll_value(lines);
        asc_print(asc_internal->file, l);
        lines = ll_next(lines);
    }
}