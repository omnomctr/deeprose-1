#include <stdlib.h>
#include "builtin.h"

#define LASSERT(args, cond, fmt, ...) \
  if (!(cond)) { \
    lval* err = lval_err(fmt, ##__VA_ARGS__); \
    lval_del(args); \
    return err; \
  }

// create lisp function, add it to the environment e, and free up the lisp values
void lenv_add_builtin(lenv* e, char* name, lbuiltin func) {
    lval* k = lval_sym(name);
    lval* v = lval_fun(func);
    lenv_put(e, k, v);
    lval_del(k);
    lval_del(v);
}

// all the functions we are adding by default
void lenv_add_builtins(lenv* e) {
    // list functions 
    lenv_add_builtin(e, "list", builtin_list);
    lenv_add_builtin(e, "first", builtin_first);
    lenv_add_builtin(e, "rest", builtin_rest);
    lenv_add_builtin(e, "eval", builtin_eval);
    lenv_add_builtin(e, "join", builtin_join);
    lenv_add_builtin(e, "cons", builtin_cons);
    lenv_add_builtin(e, "count", builtin_count);

    // math functions 
    lenv_add_builtin(e, "+", builtin_add);
    lenv_add_builtin(e, "-", builtin_sub);
    lenv_add_builtin(e, "*", builtin_mul);
    lenv_add_builtin(e, "/", builtin_div);
    lenv_add_builtin(e, "^", builtin_pow);
    lenv_add_builtin(e, "\%", builtin_mod);

    // other 
    lenv_add_builtin(e, "def", builtin_def);
}

// interface for lenv_add_builtin
lval* builtin_add(lenv* e, lval* a) {
    return builtin_operator(e, a, "+");
}

lval* builtin_sub(lenv* e, lval* a) {
    return builtin_operator(e, a, "-");
}

lval* builtin_mul(lenv* e, lval* a) {
    return builtin_operator(e, a, "*");
}

lval* builtin_div(lenv* e, lval* a) {
    return builtin_operator(e, a, "/");
}

lval* builtin_pow(lenv* e, lval* a) {
    return builtin_operator(e, a, "^");
}

lval* builtin_mod(lenv* e, lval* a) {
    return builtin_operator(e, a, "\%");
}

// all the math functions
lval* builtin_operator(lenv* e, lval* v, char* op) {
    for (int i = 0; i < v->count; i++) {
        if (v->cell[i]->type != LVAL_NUM) {
            lval_del(v);
            return lval_err("Cannot operate on a non-number");
        }
    }

    // pop out the first element
    lval* x = lval_pop(v, 0);

    //checks for negative numbers (ex: (- 5))
    if ((strcmp(op, "-") == 0) && v->count == 0) {
        x->num = -x->num;
    }

    while (v->count > 0) {
        lval* y = lval_pop(v, 0);

        if (strcmp(op, "+") == 0) { x->num += y->num; }
        if (strcmp(op, "-") == 0) { x->num -= y->num; }
        if (strcmp(op, "*") == 0) { x->num *= y->num; }
        if (strcmp(op, "\%") == 0){ x->num %= y->num; }
        if (strcmp(op, "^") == 0) { x->num = powl(x->num, y->num); }
        if (strcmp(op, "/") == 0) { 
            if (y->num == 0) {
                lval_del(x); lval_del(y);
                x = lval_err("can't divide by zero");
                break;
            } 
            x->num /= y->num; 
        }

        lval_del(y);
    }

    lval_del(v);
    return x;
}

// gives the first element of the list back in a qexpr
lval* builtin_first(lenv* e, lval* l) {
    // check for potential errors
    LASSERT(l, l->count == 1, 
        "Function 'first' passed too many arguments | Got %i, expected 1", l->count);
    LASSERT(l, l->cell[0]->type == LVAL_QEXPR, 
        "Function 'first' passed wrong type | got %s, expected %s",
        ltype_name(l->cell[0]->type), ltype_name(LVAL_QEXPR));
    LASSERT(l, l->cell[0]->count != 0, "Function 'first' passed {}");

    // take the first element
    lval* v = lval_take(l, 0);

    // delete all the other elements
    while (v->count > 1) lval_del(lval_pop(v, 1));
    return v;
}

// gives you all but the first element of a list in a qexpr
// makes more sense with linked lists and recursion trust me
lval* builtin_rest(lenv* e, lval* l) {
    // check for potential errors 
    LASSERT(l, l->count == 1, 
        "Function 'rest' passed too many arguments | got %d, expected 1",
        l->count);
    LASSERT(l, l->cell[0]->type == LVAL_QEXPR, 
        "Function 'rest' passed wrong type | got %s, expected %s", 
        ltype_name(l->cell[0]->type), ltype_name(LVAL_QEXPR));
    LASSERT(l, l->cell[0]->count != 0, "Function 'rest' passed {}");
  
    // take the first element
    lval* v = lval_take(l, 0);
    lval_del(lval_pop(v, 0));

    return v;
}

// literally just switches the lval type to a qexpr
lval* builtin_list(lenv* e, lval* l) {
    l->type = LVAL_QEXPR;
    return l;
}

// switches a qexpr to an sexpr, evaluating it
lval* builtin_eval(lenv* e, lval* l) {
    LASSERT(l, l->count == 1, 
        "Function 'eval' passed too many arguments | got %d, expected 1",
        l->count);
    LASSERT(l, l->cell[0]->type == LVAL_QEXPR, 
        "Function 'eval' passed incorrect type | got %s, expected %s",
        ltype_name(l->cell[0]->type), ltype_name(LVAL_QEXPR));

    lval* x = lval_take(l, 0);
    x->type = LVAL_SEXPR;
    return lval_eval(e, x);
}

// joins two lists
lval* builtin_join(lenv* e, lval* l) {
    for (int i = 0; i < l->count; i++) {
        LASSERT(l, l->cell[i]->type == LVAL_QEXPR, 
            "Function 'join' passed incorrect type | got %s, expected %s",
            ltype_name(l->cell[i]->type), ltype_name(LVAL_QEXPR));
    }

    lval* x = lval_pop(l, 0);

    while (l->count) {
        x = lval_join(x, lval_pop(l, 0));
    }

    lval_del(l);
    return x;
}

// kind of broken right now if you try to define a variable with a cons result 
// it will double free something 
// 
// adds an element to the front of a list (makes more sense with linked lists since its 0(n))
lval* builtin_cons(lenv* e, lval* l) {
    LASSERT(l, l->count == 2, 
        "Function 'cons' passed incorrect number of arguments | got %d, expected 2",
        l->count);
    LASSERT(l, l->cell[0]->type == LVAL_NUM, 
        "Function 'cons' passed incorrect type | got %s, expected %s",
        ltype_name(l->cell[0]->type), ltype_name(LVAL_NUM));
    LASSERT(l, l->cell[1]->type == LVAL_QEXPR, 
        "Function 'cons' passed incorrect type | got %s, expected %s",
        ltype_name(l->cell[1]->type), ltype_name(LVAL_QEXPR));

    lval* x = lval_pop(l, 0);
    lval* y = lval_pop(l, 0);

    LASSERT(l, y->type == LVAL_QEXPR, 
        "Function 'cons' passed incorrect type | got %s, expected %s",
        ltype_name(y->type), ltype_name(LVAL_QEXPR));
    
    y->count++;
    y->cell = realloc(y->cell, sizeof(lval*) * y->count);
    
    memmove(&y->cell[1], &y->cell[0], sizeof(lval*) * y->count);
    
    y->cell[0]->num = x->num;
    return y;
    
}

// returns the length of a list
lval* builtin_count(lenv* e, lval* l) {
    LASSERT(l, l->count == 1, 
        "Function 'count' passed incorrect number of arguments | got %d, expected 1",
        l->count);
    LASSERT(l, l->cell[0]->type == LVAL_QEXPR, 
        "Function 'count' passed incorrect type | got %s, expected %s",
        ltype_name(l->cell[0]->type), ltype_name(LVAL_QEXPR));

    return lval_num(l->cell[0]->count);
}

// bind a value / function to a symbol. 
// takes 1+ symbols in a qexpr, and then a list for each symbol to bind
// uses lenv_put (basically a bootleg hashmap) to mutate the environment
lval* builtin_def(lenv* e, lval* a) {
    LASSERT(a, a->cell[0]->type == LVAL_QEXPR, 
        "Function 'def' passed incorrect type | got %s, expected %s",
        ltype_name(a->cell[0]->type), ltype_name(LVAL_QEXPR));

    lval* symbols = a->cell[0];

    // check that everything is a symbol 
    for (int i = 0; i < symbols->count; i++) {
        LASSERT(a, symbols->cell[i]->type == LVAL_SYM,
             "Function 'def' cannot define %s",
             ltype_name(symbols->cell[i]->type));
    }    
    
    // check that we have the correct number of symbols and values 
    LASSERT(a, symbols->count == a->count - 1, 
        "Function 'def' cannot define incorrect number of values to symbols | got %d, expected %d",
        symbols->count, a->count - 1);

    // if it all works we can assign copies of values to symbols 
    for (int i = 0; i < symbols->count; i++) {
        lenv_put(e, symbols->cell[i], a->cell[i + 1]);
    }

    lval_del(a);

    // return empty unit () sexpr
    return lval_sexpr();
}