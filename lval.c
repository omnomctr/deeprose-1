#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdarg.h>
#include "lval.h"


lval* lval_num(long x) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_NUM;
    v->num = x;
    return v;
}

lval* lval_err(char* message) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_ERR;
    v->err = malloc(strlen(message) + 1);
    strcpy(v->err, message);
    return v;
}

lval* lval_sym(char* symbol) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_SYM;
    v->sym = malloc(sizeof(symbol) + 1);
    strcpy(v->sym, symbol);
    return v;
}

// returns a pointer to a new empty s-expression
lval* lval_sexpr(void) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_SEXPR;
    v->count = 0;
    v->cell = NULL;
    return v;
}

lval* lval_fun(lbuiltin func) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_FUN;
    v->fun = func;
    return v;
}

void lval_del(lval* v) {
    switch (v->type) {
        case LVAL_NUM: break;

        case LVAL_ERR: free(v->err); break;
        case LVAL_SYM: free(v->sym); break;

        case LVAL_FUN: break;
        // if its a qexpr or a sexpr we need to recursively delete its elements
        case LVAL_QEXPR:
        case LVAL_SEXPR:
            //we need to manually free up all the s expressions children using recursion
            for (int i = 0; i < v->count; i++) {
                lval_del(v->cell[i]);
            }

            break;
    }

    free(v);
}

lval* lval_read_num(mpc_ast_t* t) {
    // errno is basically c's error handling that isn't -1 
    // its a macro that expands into a global or something idk
    errno = 0;
    long x = strtol(t->contents, NULL, 10);
    return errno != ERANGE ?
        lval_num(x) : lval_err("invalid number");
}


lval* lval_read(mpc_ast_t* t) {
    // if its a number or symbol we can just return the converted type 
    if (strstr(t->tag, "number")) { return lval_read_num(t); }
    if (strstr(t->tag, "symbol")) { return lval_sym(t->contents); }
    

    lval* x = NULL;
    // > is the root according to the parser
    if (strcmp(t->tag, ">") == 0) { x = lval_sexpr(); }
    if (strcmp(t->tag, "sexpr"))  { x = lval_sexpr(); }
    if (strstr(t->tag, "qexpr"))  { x = lval_qexpr(); }

    for (int i = 0; i < t->children_num; i++) {
        if (strcmp(t->children[i]->contents, "(") == 0) { continue; }
        if (strcmp(t->children[i]->contents, ")") == 0) { continue; }
        if (strcmp(t->children[i]->contents, "{") == 0) { continue; }
        if (strcmp(t->children[i]->contents, "}") == 0) { continue; }
        if (strcmp(t->children[i]->tag , "regex") == 0) { continue; }

        x = lval_add(x, lval_read(t->children[i]));
    }

    return x;
}

lval* lval_add(lval* v, lval* x) {
    v->count++;
    v->cell = realloc(v->cell, sizeof(lval*) * v->count);
    v->cell[v->count - 1] = x;
    return v;
}

void lval_expr_print(lval* v, char open, char close) {
    putchar(open);

    for(int i = 0; i < v->count; i++) {
        lval_print(v->cell[i]);

        if (i != (v->count - 1)) {
            putchar(' ');
        }
    }

    putchar(close);
}

void lval_print(lval* v) {
    switch (v->type) {
        case LVAL_NUM:   printf("%li", v->num); break;
        case LVAL_ERR:   printf("Error: %s", v->err); break;
        case LVAL_SYM:   printf("%s", v->sym); break;
        case LVAL_SEXPR: lval_expr_print(v, '(', ')'); break;
        case LVAL_QEXPR: lval_expr_print(v, '{', '}'); break;
        case LVAL_FUN:   printf("<function>"); break;
    }
}

void lval_println(lval* v) { lval_print(v); putchar('\n'); }

lval* lval_copy(lval* v) {
    lval* x = malloc(sizeof(lval));
    x->type = v->type;

    switch (v->type) {
        case LVAL_NUM: x->num = v->num; break;
        case LVAL_FUN: x->fun = v->fun; break;

        // copying strings using malloc and strcpy
        case LVAL_ERR: 
            x->err = malloc(strlen(v->err) + 1);
            strcpy(x->err, v->err);
            break;
        case LVAL_SYM:
            x->sym = malloc(strlen(v->sym) + 1);
            strcpy(x->sym, v->sym);
            break;

        case LVAL_QEXPR:
        case LVAL_SEXPR:
            x->count = v->count;
            x->cell = malloc(sizeof(lval*) * x->count);
            for (int i = 0; i < x->count; i++) {
                x->cell[i] = lval_copy(v->cell[i]);
            }
        break;
    }
    return x;
}

lval* lval_eval_sexpr(lenv* e, lval* v) {
    // recursively evaluate children 
    for (int i = 0; i < v->count; i++) {
        v->cell[i] = lval_eval(e, v->cell[i]);
    }

    // error checking 
    for (int i = 0; i < v->count; i++) {
        if (v->cell[i]->type == LVAL_ERR) { return lval_take(v, i); }
    }

    // check for empty expr 
    if (v->count == 0) { return v; }

    // check for single expr
    if (v->count == 1) { return lval_take(v, 0); }

    // check that first element is a symbol 
    lval* f = lval_pop(v, 0);
    if (f->type != LVAL_FUN) {
        lval_del(f); lval_del(v);
        return lval_err("First element of S-expression is not a function");
    }

    // evaluate it with a builtin operator 
    lval* result = f->fun(e, v);
    lval_del(f);
    return result;
}


lval* lval_eval(lenv* e, lval* v) {
    if (v->type == LVAL_SYM) {
        lval* x = lenv_get(e, v);
        lval_del(v);
        return x;
    }
    if (v->type == LVAL_SEXPR) { return lval_eval_sexpr(e, v); }
    return v;
}


lval* lval_pop(lval* v, int i) {
    lval* x = v->cell[i];

    // shift everything to envelope x
    memmove(&v->cell[i], &v->cell[i + 1], 
        sizeof(lval*) * (v->count - i - 1));

    v->count--;

    v->cell = realloc(v->cell, sizeof(lval*) * v->count);
    return x;
}

// same as lval_pop but it consumes the lval ptr passed in
lval* lval_take(lval* v, int i) {
    lval* x = lval_pop(v, i);
    lval_del(v);
    return x;
}



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

lval* lval_join(lval* x, lval* y) {
  while (y->count) {
    x = lval_add(x, lval_pop(y, 0));
  }
  lval_del(y);
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



lval* lval_qexpr(void) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_QEXPR;
    v->count = 0;
    v->cell = NULL;
    return v;
}

lenv* lenv_new(void) {
    lenv* e = malloc(sizeof(lenv));
    e->count = 0;
    e->syms = NULL;
    e->vals = NULL;
    return e;
}

void lenv_del(lenv* e) {
    for (int i = 0; i < e->count; i++) {
        free(e->syms[i]);
        lval_del(e->vals[i]);
    }
    free(e->syms);
    free(e->vals);
    free(e);
}

lval* lenv_get(lenv* e, lval* key) {
    // checks if any items match k in the lenv e 
    for (int i = 0; i < e->count; i++) {
        if (strcmp(e->syms[i], key->sym) == 0) {
            return lval_copy(e->vals[i]);
        }
    }

    // otherwise we can return an error 
    return lval_err("Unbound Symbol");
}

void lenv_put(lenv* e, lval* key, lval* value) {
    // check if variable already exists
    for (int i = 0; i < e->count; i++) {
        // if so, replace it with the new value 
        if (strcmp(e->syms[i], key->sym) == 0) {
            lval_del(e->vals[i]);
            e->vals[i] = lval_copy(value);
        }
    }

    // if not we have to allocate everything and such
    e->count++;
    e->vals = realloc(e->vals, sizeof(lval*) * e->count);
    e->syms = realloc(e->syms, sizeof(char*) * e->count);

    e->vals[e->count - 1] = lval_copy(value);
    e->syms[e->count - 1] = malloc(strlen(key->sym) + 1);
    strcpy(e->syms[e->count - 1], key->sym);
}

