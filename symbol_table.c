#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "types.h"
#include "symbol_table.h"
#include "stack.h"
#include "linked_list.h"

static SYMBOL_TABLE* symbols;

SYMBOL_TABLE* get_symbol_table() {
  if(!symbols) {
    symbols = malloc(sizeof(SYMBOL_TABLE));
    symbols->current_scope = NULL;
  }
  return symbols;
}

SCOPE* new_scope(SCOPE* parent, int id)
{
  SCOPE *new = malloc(sizeof(SCOPE));

  new->id = id;
  new->children = NULL;
  new->symbols = NULL;
  new->parent = parent;

  if(parent) {
    parent->children = ll_insertfront(parent->children, new);
  }

  //printf("New scope %p is a child of %p\n", new, parent);
  return new;
}

SCOPE* exit_scope(SCOPE* current) 
{
  if(!current) {
    return NULL;
  }
  //printf("Leaving scope %p to %p\n", current, current->parent);
  return current->parent;
}

/*
    When we add to a scope, we're nesting inward. However, to check if
    something exists in a scope, we have to check the symbol table of
    every parent if it doesn't exist in the current scope.
*/
SYMBOL* find_in_scope(SCOPE* s, char* target) {

  if(s == NULL) {
    return NULL;
  }

  SYMBOL* sym = find_entry(s->symbols, target);

  if(sym == NULL) {
    sym = find_in_scope(s->parent, target);
  }

  return sym;
}

/*
  Searches down a symbol table from a parent node
*/
SYMBOL* find_in_children(SCOPE* s, char* target)
{
  if(s == NULL) {
    return NULL;
  }

  SYMBOL* sym = find_entry(s->symbols, target);

  if(sym == NULL) {
   LINKED_LIST* child = s->children;
    while(child) {
      sym = find_in_children(ll_value(child), target);
      if (sym != NULL) {
        return sym;
      }
      child = ll_next(child);
    }
  }

  return sym;
}

SYMBOL* new_symbol(SYMTYPE* type, char* name, int meta, char* extra) {
  SYMBOL* insertNew  = malloc(sizeof(SYMBOL));
  
  if(extra) { insertNew->extra = strdup(extra); } 

  insertNew->name = strdup(name);
  insertNew->meta = meta;
  insertNew->type = type;
  insertNew->stack_offset = 0;
  insertNew->registers = 0;
  insertNew->on_stack = false;
  insertNew->scope = NULL;

  return insertNew;
}

LINKED_LIST* add_symbols_to_scope(SCOPE* scope, LINKED_LIST* symbols)
{
  if(!scope) {
    return NULL;
  }

  scope->symbols = ll_combine(scope->symbols, symbols);

  return scope->symbols;
}

bool name_match_symbol(LINKED_LIST* l, void* name) {
  SYMBOL* s = ll_value(l);
  return s && s->name && strcmp(s->name, name) == 0;
}

SYMBOL* find_entry(LINKED_LIST* start, char* name)
{
  return ll_find(start, name, name_match_symbol);
}

bool size_comparator(LINKED_LIST* a, LINKED_LIST* b) {
  SYMBOL* a_val = ll_value(a);
  SYMBOL* b_val = ll_value(b);

  return get_type_width(a_val->type) < get_type_width(b_val->type) ? false : true;
}

void reorder_symbols(SCOPE* s) {
  if(s) {
    ll_mergesort(&s->symbols, size_comparator);
  }
}

void print_symbol_table(SYMBOL_TABLE* st, FILE* f) {
  if(st) {
    print_scope(st->current_scope, f);
  }
}

void print_scope(SCOPE* sc, FILE* f) {
  
  if(!sc) {
    return;
  }

  LINKED_LIST* s = sc->symbols;
  
  while(s) {
    SYMBOL* symbol = ll_value(s);
    print_symbol(symbol, sc->id, f);
    s = ll_next(s);
  }
  
  LINKED_LIST* children = sc->children;

  while(children) {
    SCOPE* scope = ll_value(children);
    print_scope(scope, f);
    children = ll_next(children);
  }
}

void print_symbol(SYMBOL* symbol, int scope, FILE* f) {
  if(!symbol) {
    return;
  }

  if(f) {
    fprintf(f, "%s : %d : %s : %s\n", symbol->name, scope,  symbol->type ? symbol->type->name : "NULL", symbol->extra);
  } else {
    printf("%s : %d : %s : %s\n", symbol->name, scope, symbol->type ? symbol->type->name : "NULL", symbol->extra);
  }

}

int pad_4(int original) {
    int mod = original % 4;
    if(mod > 0) {
        original += (4 - mod);
    }
    return original;
}

int compute_stack_space(SCOPE* s, int base_offset) {

    if(!s) {
        return base_offset;
    }

    reorder_symbols(s);

    LINKED_LIST* syms = s->symbols;

    int space_needed = base_offset;

    while(syms) {
        SYMBOL* s = ll_value(syms);
        s->stack_offset = space_needed;
        //printf("Offset for %s: %d\n", s->name, space_needed);
        space_needed += get_type_width(s->type);
        syms = ll_next(syms);
    }

    space_needed = pad_4(space_needed);

    LINKED_LIST* children = s->children;
    
    int children_max = space_needed;
    
    while(children) {
        SCOPE* child_s = ll_value(children);
        int width = compute_stack_space(child_s, space_needed);
        
        if(width > children_max) {
            children_max = width;
        }

        children = ll_next(children);
    }

    s->stack_space = children_max;
    // printf("Stack space for %d is: %d\n", s->id, children_max);
    return children_max;
}

int compute_param_space(SCOPE* s, SYMBOL* ret) {

    if(!s) {
        return 0;
    }

    LINKED_LIST* symbols = s->symbols;

    int end = 0;

    while(symbols) {
        SYMBOL* s = ll_value(symbols);
        s->stack_offset = end;
        end += get_type_width(s->type);
        end = pad_4(end);
        symbols = ll_next(symbols);
    }

    // RETURN, CONTROL, ACCESS, PC
    int ret_width = pad_4(get_type_width(ret->type));
    ret->stack_offset = -12 - ret_width;
    end += ret_width;

    symbols = s->symbols;

    while(symbols) {
        SYMBOL* s = ll_value(symbols);
        s->stack_offset -= (end + 12);
        // printf("%s offset is %d\n", s->name, s->stack_offset);
        symbols = ll_next(symbols);
    }

    s->stack_space = end;

    return end;
}