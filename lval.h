#ifndef LVAL_HEADER
#define LVAL_HEADER
#include <stdarg.h>
#include "mpc.h"

struct lval;
struct lenv;
typedef struct lval lval;
typedef struct lenv lenv;

enum lisptype { LVAL_NUM, LVAL_ERR, LVAL_SYM, LVAL_STR, LVAL_SEXPR, LVAL_QEXPR, LVAL_FUN }; // type enum
enum lisperror { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM }; // error type enum

typedef lval*(*lbuiltin)(lenv*, lval*);

// lisp value struct 
struct lval {
    int type;

    long num;
    // attached strings
    char* err;
    char* sym;
    char* str;    
    
    // function 
    lbuiltin builtin;
    lenv* env;
    lval* formals;
    lval* body;
    
    // other lisp values in the list
    int count;
    struct lval** cell;
};

struct lenv {
    lenv* parent;
    int count;
    char** syms;
    lval** vals;
};

char* ltype_name(int t);
lval* lval_num(long x);
lval* lval_err(char* fmt, ...);
lval* lval_sym(char* symbol);
lval* lval_sexpr(void);
lval* lval_str(char* str);
lval* lval_fun(lbuiltin func);
void lval_del(lval* v);
lval* lval_copy(lval* v);
lval* lval_read_num(mpc_ast_t* t);
lval* lval_add(lval* v, lval* x);
lval* lval_read(mpc_ast_t* t);
lval* lval_read_str(mpc_ast_t* t);
void lval_print(lval* v);
void lval_println(lval* v);
void lval_expr_print(lval* v, char* open, char* close);
void lval_print_str(lval* v);
lval* lval_eval_sexpr(lenv* e, lval* v);
lval* lval_take(lval* v, int i);
lval* lval_pop(lval* v, int i);
lval* lval_eval(lenv* e, lval* v);
lval* lval_qexpr(void);
lval* lval_call(lenv* e, lval* f, lval* a);

lval* lval_join(lval* x, lval* y);
lval* lval_lambda(lval* formals, lval* body);
#endif