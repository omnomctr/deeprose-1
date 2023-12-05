#ifndef FUNCTIONS_H_INCLUDED
#define FUNCTIONS_H_INCLUDED
#include <stdarg.h>
#include "mpc.h"

struct lval;
struct lenv;
typedef struct lval lval;
typedef struct lenv lenv;

enum { LVAL_NUM, LVAL_ERR, LVAL_SYM, LVAL_SEXPR, LVAL_QEXPR, LVAL_FUN }; // type enum
enum lisperror { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM }; // error type enum

typedef lval*(*lbuiltin)(lenv*, lval*);

// lisp value struct 
struct lval {
    int type;

    long num;
    // attached strings
    char* err;
    char* sym;
        
    // function 
    lbuiltin fun;
    
    // other lisp values in the list
    int count;
    struct lval** cell;
};



struct lenv {
    int count;
    char** syms;
    lval** vals;
};


lval* lval_num(long x);
lval* lval_err(char* message);
lval* lval_sym(char* symbol);
lval* lval_sexpr(void);
lval* lval_fun(lbuiltin func);
void lval_del(lval* v);
lval* lval_copy(lval* v);
lval* lval_read_num(mpc_ast_t* t);
lval* lval_add(lval* v, lval* x);
lval* lval_read(mpc_ast_t* t);
void lval_print(lval* v);
void lval_println(lval* v);
void lval_expr_print(lval* v, char open, char close);
lval* lval_eval_sexpr(lenv* e, lval* v);
lval* lval_take(lval* v, int i);
lval* lval_pop(lval* v, int i);
lval* lval_eval(lenv* e, lval* v);
lval* lval_qexpr(void);

lenv* lenv_new(void);
void lenv_del(lenv* e);
lval* lenv_get(lenv* e, lval* key);
void lenv_put(lenv* e, lval* key, lval* value);

void lenv_add_builtin(lenv* e, char* name, lbuiltin func);
void lenv_add_builtins(lenv* e);

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

lval* lval_join(lval* x, lval* y);
#endif