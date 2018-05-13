#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol_table.h"
#include "asc.h"
#include "stack.h"

#define MAX_LINE 1024

static ASC* asc_internal = NULL;

void asc_init(FILE* f) {
    if(asc_internal) {
        //free_asc(asc_internal);
    }

    asc_internal = malloc(sizeof(ASC));
    asc_internal->file = f;
    asc_internal->lines = NULL;
    asc_internal->scope = NULL;
    asc_new_line(1);
}

void asc_error(char* e) {
    asc_new_line(-1);
    asc_append(e);
}

void asc_line_scope(SCOPE* s) {
    if(!asc_internal || !s) {
        return;
    }
    LINE* l = ll_value(asc_internal->lines);
    l->scope = s;
}

void asc_new_line(int n) {

    if(!asc_internal) {
        return;
    }

    LINE* l = malloc(sizeof(LINE));
    l->lineno = n;
    l->text = malloc(MAX_LINE * sizeof(char));
    l->scope = NULL;
    memset(l->text, 0, MAX_LINE);

    asc_internal->lines = ll_insertfront(asc_internal->lines, l);
}

char* scope_str(SCOPE* s) {

    if(!s) {
        return NULL;
    }

    STACK* scopes = NULL;
    int count = 0;

    while(s) {
        scopes = stack_push(scopes, s);
        s = s->parent;
        count++;
    }

    char* scope_str = malloc((count * 16) * sizeof(char));
    memset(scope_str, 0, count * 16);

    while(scopes) {
        SCOPE* sc = stack_peek(scopes);
        char single[16];
        sprintf(single, "%d:", sc->id);
        strcat(scope_str, single);
        scopes = stack_pop(scopes);
    }

    return scope_str;
}

void asc_print(FILE* f, LINE* l) {
    if(!f || !l) {
        return;
    }

    if(l->lineno >= 0) {
        fprintf(f, "%d:", l->lineno);
        char* scopes = scope_str(l->scope);
        if(scopes) {
            fprintf(f, "%s", scopes);
            free(scopes);
        }
        fprintf(f, "\t\t\t %s\n", l->text);
    } else {
        fprintf(f, "%s\n", l->text);
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

    char* token = strtok(str, "\n\r");

    if(!token) {
        strcat(l->text, str);
    } else {
        int lineno = l->lineno;
        strcat(l->text, token);
        token = strtok(NULL, "\n\r");
        while(token != NULL)  {
            asc_new_line(++lineno);
            LINE* l = ll_value(asc_internal->lines);
            strcat(l->text, token);
            token = strtok(NULL, "\n\r");
        }
    }
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