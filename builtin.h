#ifndef FUNCTIONS_INCLUDED_1
#define FUNCTIONS_INCLUDED_1
#include "lval.h"


extern void lenv_add_builtin(lenv* e, char* name, lbuiltin func);
extern void lenv_add_builtins(lenv* e);

lval* builtin(lenv* e, lval* a, char* func);
lval* builtin_operator(lenv* e, lval* v, char* op);
lval* builtin_add(lenv* e, lval* a);
lval* builtin_sub(lenv* e, lval* a);
lval* builtin_mul(lenv* e, lval* a);
lval* builtin_div(lenv* e, lval* a);
lval* builtin_pow(lenv* e, lval* a);
lval* builtin_first(lenv* e, lval* l);
lval* builtin_rest(lenv* e, lval* l);
lval* builtin_list(lenv* e, lval* l);
lval* builtin_eval(lenv* e, lval* l);
lval* builtin_join(lenv* e, lval* l);
lval* builtin_cons(lenv* e, lval* l);
lval* builtin_count(lenv* e, lval* l);
lval* builtin_def(lenv* e, lval* a);
#endif