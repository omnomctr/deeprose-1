#include <stdlib.h>
#include "builtin.h"

#define LASSERT(args, cond, err) \
    if (!(cond)) { lval_del(args); return lval_err(err); }

void lenv_add_builtin(lenv* e, char* name, lbuiltin func) {
    lval* k = lval_sym(name);
    lval* v = lval_fun(func);
    lenv_put(e, k, v);
    lval_del(k);
    lval_del(v);
}

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

    // other 
    lenv_add_builtin(e, "def", builtin_def);
}

lval* builtin(lenv* e, lval* a, char* func) {
    if (strcmp("list", func) == 0) { return builtin_list(e, a); }
    if (strcmp("first", func) == 0) { return builtin_first(e, a); }
    if (strcmp("rest", func) == 0) { return builtin_rest(e, a); }
    if (strcmp("join", func) == 0) { return builtin_join(e, a); }
    if (strcmp("eval", func) == 0) { return builtin_eval(e, a); }
    if (strcmp("cons", func) == 0) { return builtin_cons(e, a); }
    if (strcmp("count", func) == 0) {return builtin_count(e, a); }
    if (strstr("+-/*", func)) { return builtin_operator(e, a, func); }
    lval_del(a);
    return lval_err("Unknown Function!");
}

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

lval* builtin_first(lenv* e, lval* l) {
    // check for potential errors
    LASSERT(l, l->count == 1, "Function 'first' passed too many arguments");
    LASSERT(l, l->cell[0]->type == LVAL_QEXPR, "Function 'first' passed wrong type");
    LASSERT(l, l->cell[0]->count != 0, "Function 'first' passed {}");

    // take the first element
    lval* v = lval_take(l, 0);

    // delete all the other elements
    while (v->count > 1) lval_del(lval_pop(v, 1));
    return v;
}

lval* builtin_rest(lenv* e, lval* l) {
    // check for potential errors 
    LASSERT(l, l->count == 1, "Function 'rest' passed too many arguments");
    LASSERT(l, l->cell[0]->type == LVAL_QEXPR, "Function 'rest' passed wrong type");
    LASSERT(l, l->cell[0]->count != 0, "Function 'rest' passed {}");
  
    // take the first element
    lval* v = lval_take(l, 0);
    lval_del(lval_pop(v, 0));

    return v;
}

lval* builtin_list(lenv* e, lval* l) {
    l->type = LVAL_QEXPR;
    return l;
}

lval* builtin_eval(lenv* e, lval* l) {
    LASSERT(l, l->count == 1, "Function 'eval' passed too many arguments");
    LASSERT(l, l->cell[0]->type == LVAL_QEXPR, "Function 'eval' passed incorrect type");

    lval* x = lval_take(l, 0);
    x->type = LVAL_SEXPR;
    return lval_eval(e, x);
}

lval* builtin_join(lenv* e, lval* l) {
    for (int i = 0; i < l->count; i++) {
        LASSERT(l, l->cell[i]->type == LVAL_QEXPR, "Function 'join' passed incorrect type");
    }

    lval* x = lval_pop(l, 0);

    while (l->count) {
        x = lval_join(x, lval_pop(l, 0));
    }

    lval_del(l);
    return x;
}


lval* builtin_cons(lenv* e, lval* l) {
    LASSERT(l, l->count == 2, "Function 'cons' passed incorrect number of arguments");
    LASSERT(l, l->cell[0]->type == LVAL_NUM, "Function 'cons' passed incorrect type");
    LASSERT(l, l->cell[1]->type == LVAL_QEXPR, "Function 'cons' passed incorrect type");

    lval* x = lval_pop(l, 0);
    lval* y = lval_pop(l, 0);

    LASSERT(l, y->type == LVAL_QEXPR, "Function 'cons' passed incorrect type");
    
    y->count++;
    y->cell = realloc(y->cell, sizeof(lval*) * y->count);
    
    memmove(&y->cell[1], &y->cell[0], sizeof(lval*) * y->count);
    
    y->cell[0]->num = x->num;
    return y;
    
}

lval* builtin_count(lenv* e, lval* l) {
    LASSERT(l, l->count == 1, "Function 'count' passed incorrect number of arguments");
    LASSERT(l, l->cell[0]->type == LVAL_QEXPR, "Function 'count' passed incorrect type");

    return lval_num(l->cell[0]->count);
}

lval* builtin_def(lenv* e, lval* a) {
    LASSERT(a, a->cell[0]->type == LVAL_QEXPR, "Function 'def' passed incorrect type");

    lval* symbols = a->cell[0];

    // check that everything is a symbol 
    for (int i = 0; i < symbols->count; i++) {
        LASSERT(a, symbols->cell[i]->type == LVAL_SYM, "Function 'def' cannot define new symbol");
    }

    // check that we have the correct number of symbols and values 
    LASSERT(a, symbols->count == a->count - 1, 
        "Function 'def' cannot define incorrect number of values to symbols");

    // if it all works we can assign copies of values to symbols 
    for (int i = 0; i < symbols->count; i++) {
        lenv_put(e, symbols->cell[i], a->cell[i + 1]);
    }

    lval_del(a);


    return lval_sexpr();
}