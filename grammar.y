%{
#include <stdio.h>

%}

%union {
        int id;
        int number;
        double real;
        int boolean;
}

%start program;
%token <number> number;
%token <id> identifier;

%%

program: 
        definition_list sblock
        ;

definition_list:
        | definition
        | definition definition_list 
        ;

%%

