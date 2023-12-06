#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdarg.h>
#include "lval.h"

// returns LVAL enum's string name
char* ltype_name(int t) {
  switch(t) {
    case LVAL_FUN: return "Function";
    case LVAL_NUM: return "Number";
    case LVAL_ERR: return "Error";
    case LVAL_SYM: return "Symbol";
    case LVAL_SEXPR: return "S-Expression";
    case LVAL_QEXPR: return "Q-Expression";
    default: return "Unknown";
  }
}

// create a lisp value number
lval* lval_num(long x) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_NUM;
    v->num = x;
    return v;
}

// create a lisp value error
lval* lval_err(char* fmt, ...) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_ERR;

    va_list va;
    va_start(va, fmt);

    // should be enought
    v->err = malloc(512);

    // put the arguments into the error string with c's format!
    vsnprintf(v->err, 511, fmt, va);

    // reallocate
    v->err = realloc(v->err, strlen(v->err) + 1);

    va_end(va);
    return v;
}

// create a lisp value symbol
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

// create a lisp  value function (takes in a function ptr)
lval* lval_fun(lbuiltin func) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_FUN;
    v->fun = func;
    return v;
}

// recursively free up lisp values
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

// reads and converts the number (used in the repl)
lval* lval_read_num(mpc_ast_t* t) {
    // errno is basically c's error handling that isn't -1 
    // its a macro that expands into a global or something idk
    errno = 0;
    long x = strtol(t->contents, NULL, 10);
    return errno != ERANGE ?
        lval_num(x) : lval_err("invalid number");
}

// parse AST into lisp values
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

// add a lisp value to another lisp value
lval* lval_add(lval* v, lval* x) {
    v->count++;
    v->cell = realloc(v->cell, sizeof(lval*) * v->count);
    v->cell[v->count - 1] = x;
    return v;
}

// print out an expression recursively w/ lval_print
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

// prints out the lisp value depending on what it is
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

// adds a println to lval_print what can I say
void lval_println(lval* v) { lval_print(v); putchar('\n'); }

// create a copy of an lval 
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

// evaluate s-expression 
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

// evaluate symbols and then give them to lval_eval_sexpr
lval* lval_eval(lenv* e, lval* v) {
    if (v->type == LVAL_SYM) {
        lval* x = lenv_get(e, v);
        lval_del(v);
        return x;
    }
    if (v->type == LVAL_SEXPR) { return lval_eval_sexpr(e, v); }
    return v;
}

// takes out the i'th element from an array, moving everything 
// to account for it.
lval* lval_pop(lval* v, int i) {
    lval* x = v->cell[i];

    // shift everything to envelope x
    memmove(&v->cell[i], &v->cell[i + 1], 
        sizeof(lval*) * (v->count - i - 1));

    v->count--;

    v->cell = realloc(v->cell, sizeof(lval*) * v->count);
    return x;
}

// same as lval_pop but it also frees up the lval* passed in
lval* lval_take(lval* v, int i) {
    lval* x = lval_pop(v, i);
    lval_del(v);
    return x;
}

// joins two lvals. frees y
lval* lval_join(lval* x, lval* y) {
  while (y->count) {
    x = lval_add(x, lval_pop(y, 0));
  }
  lval_del(y);
  return x;
}

// create new q-expr (like s-expr but not evaluated)
lval* lval_qexpr(void) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_QEXPR;
    v->count = 0;
    v->cell = NULL;
    return v;
}

// new environment
lenv* lenv_new(void) {
    lenv* e = malloc(sizeof(lenv));
    e->count = 0;
    e->syms = NULL;
    e->vals = NULL;
    return e;
}

// delete environment
void lenv_del(lenv* e) {
    for (int i = 0; i < e->count; i++) {
        free(e->syms[i]);
        lval_del(e->vals[i]);
    }
    free(e->syms);
    free(e->vals);
    free(e);
}

// get a symbol's value from the environment. 
// returns an LVAL_ERR if it cant find it
lval* lenv_get(lenv* e, lval* key) {
    // checks if any items match k in the lenv e 
    for (int i = 0; i < e->count; i++) {
        if (strcmp(e->syms[i], key->sym) == 0) {
            return lval_copy(e->vals[i]);
        }
    }

    // otherwise we can return an error 
    return lval_err("Unbound Symbol '%s'", key->sym);
}

// binds a symbol to a value
void lenv_put(lenv* e, lval* key, lval* value) {
    // check if variable already exists
    for (int i = 0; i < e->count; i++) {
        // if so, delete the old version to not get weird indexing issues
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

