/// all the lisp environment functions
#include "lenv.h"

// new environment
lenv* lenv_new(void) {
    lenv* e = malloc(sizeof(lenv));
    e->parent = NULL;
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

    // if there isn't a symbol we can check the parent otherwise return an error
    return (e->parent) ? lenv_get(e->parent, key) : lval_err("Unbound symbol %s", key->sym);
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

lenv* lenv_copy(lenv* e) {
    lenv* new = malloc(sizeof(lenv));
    new->parent = e->parent;
    new->count = e->count;
    new->syms = malloc(sizeof(char*) * new->count);
    new->vals = malloc(sizeof(lval*) * new->count);
    for (int i = 0; i < e->count; i++) {
        new->syms[i] = malloc(strlen(e->syms[i]) + 1);
        strcpy(new->syms[i], e->syms[i]);

        new->vals[i] = lval_copy(e->vals[i]);
    }

    return new;
}

// for binding a value to a symbol globally
void lenv_def(lenv* e, lval* key, lval* value) {
    while (e->parent) { e = e->parent; }
    lenv_put(e, key, value);
}