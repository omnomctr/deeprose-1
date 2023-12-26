#ifndef BUILTIN_HEADER
#define BUILTIN_HEADER
#include "lval.h"
#include "lenv.h"

// a few macros I made to do error handling
#define LASSERT(args, cond, fmt, ...) \
  if (!(cond)) { \
    lval* err = lval_err(fmt, ##__VA_ARGS__); \
    lval_del(args); \
    return err; \
  }

#define LASSERT_ARGS_NUM(fnname_str, lval_ptr, num) \
    if (lval_ptr->count != num) { \
        lval* err = lval_err(   \
            "Function '%s' passed incorrect number of args | got %d, expected %d", \
            fnname_str, lval_ptr->count, num \
            ); \
        lval_del(lval_ptr); \
        return err; \
    }

#define LASSERT_ARGS_TYPE(fnname_str, lval_ptr, index, checktype) \
    if (lval_ptr->cell[index]->type != checktype) { \
        lval* err = lval_err( \
            "Function '%s' passed incorrect type | got %s, expected %s", \
            fnname_str, ltype_name(lval_ptr->cell[index]->type), ltype_name(checktype)); \
        lval_del(lval_ptr); \
        return err; \
    }

extern void lenv_add_builtin(lenv* e, char* name, lbuiltin func);
extern void lenv_add_builtins(lenv* e);

lval* builtin(lenv* e, lval* a, char* func);
lval* builtin_operator(lenv* e, lval* v, char* op);
lval* builtin_add(lenv* e, lval* a);
lval* builtin_sub(lenv* e, lval* a);
lval* builtin_mul(lenv* e, lval* a);
lval* builtin_div(lenv* e, lval* a);
lval* builtin_pow(lenv* e, lval* a);
lval* builtin_mod(lenv* e, lval* a);
lval* builtin_first(lenv* e, lval* l);
lval* builtin_rest(lenv* e, lval* l);
lval* builtin_list(lenv* e, lval* l);
lval* builtin_eval(lenv* e, lval* l);
lval* builtin_join(lenv* e, lval* l);
//lval* builtin_cons(lenv* e, lval* l);
lval* builtin_count(lenv* e, lval* l);
lval* builtin_def(lenv* e, lval* a);
lval* builtin_let(lenv* e, lval* a);
lval* builtin_lambda(lenv* e, lval* a);
lval* builtin_gt(lenv* e, lval* a);
lval* builtin_ge(lenv* e, lval* a);
lval* builtin_lt(lenv* e, lval* a);
lval* builtin_le(lenv* e, lval* a);
lval* builtin_eq(lenv* e, lval* a);

lval* builtin_and(lenv* e, lval* a);
lval* builtin_or(lenv* e, lval* a);
lval* builtin_not(lenv* e, lval* a);

lval* builtin_if(lenv* e, lval* a);

lval* builtin_load(lenv* e, lval* a);
lval* builtin_print(lenv* e, lval* a);
lval* builtin_exit(lenv* e, lval* a);
lval* builtin_error(lenv* e, lval* a);

lval* builtin_atoi(lenv* e, lval* a);
lval* builtin_itoa(lenv* e, lval* a);
lval* builtin_strtoascii(lenv* e, lval* a);
lval* builtin_asciitostr(lenv* e, lval* a);
#endif