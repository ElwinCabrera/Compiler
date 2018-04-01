#ifndef ERRORS_H
#define ERRORS_H

typedef struct error {
    char * e;
    struct error * next;
} ERROR;

ERROR * push_error(ERROR*, char*);
ERROR * pop_error(ERROR*);
void free_error(ERROR *);

#endif
