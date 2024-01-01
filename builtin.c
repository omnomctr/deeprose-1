#include <stdlib.h>
#include <time.h>
#include "builtin.h"

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
    lenv_add_builtin(e, "load", builtin_load);
    lenv_add_builtin(e, "count", builtin_count);

    // math functions 
    lenv_add_builtin(e, "+", builtin_add);
    lenv_add_builtin(e, "-", builtin_sub);
    lenv_add_builtin(e, "*", builtin_mul);
    lenv_add_builtin(e, "/", builtin_div);
    lenv_add_builtin(e, "^", builtin_pow);
    lenv_add_builtin(e, "\%", builtin_mod);

    // ordering / conditionals
    lenv_add_builtin(e, "<", builtin_lt);
    lenv_add_builtin(e, ">", builtin_gt);
    lenv_add_builtin(e, "=", builtin_eq);
    lenv_add_builtin(e, "and", builtin_and);
    lenv_add_builtin(e, "or", builtin_or);
    lenv_add_builtin(e, "not", builtin_not);
    lenv_add_builtin(e, "if", builtin_if);
    
    // defining things
    lenv_add_builtin(e, "def", builtin_def);
    lenv_add_builtin(e, "let", builtin_let);
    lenv_add_builtin(e, "\\", builtin_lambda);

    // side effects
    lenv_add_builtin(e, "print", builtin_print);
    lenv_add_builtin(e, "exit", builtin_exit);
    lenv_add_builtin(e, "error", builtin_error);
    lenv_add_builtin(e, "input-num", builtin_input_num);
    lenv_add_builtin(e, "random-number", builtin_random_number);

    // other 
    lenv_add_builtin(e, "atoi", builtin_atoi);
    lenv_add_builtin(e, "itoa", builtin_itoa);
    lenv_add_builtin(e, "strtoascii", builtin_strtoascii);
    lenv_add_builtin(e, "asciitostr", builtin_asciitostr);
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

int lval_eq(lval* x, lval* y) {
    if (x->type != y->type) return 0;

    switch (x->type){
        case LVAL_NUM:  return (x->num == y->num);  
        // comparing strings 
        case LVAL_ERR: return (strcmp(x->err, y->err) == 0);
        case LVAL_SYM: return (strcmp(x->sym, y->sym) == 0);
        case LVAL_STR: return (strcmp(x->str, y->str) == 0);

        // functions are kinda funky to compare but whatever
        case LVAL_FUN:
            if (x->builtin || y->builtin) {
                return x->builtin == y->builtin;
            } else {
                return lval_eq(x->formals, y->formals) && lval_eq(x->body, y->body);
            }
        
        case LVAL_QEXPR:
        case LVAL_SEXPR:
            if (x->count != y->count) return 0;
            for (int i = 0; i < x->count; i++) {
                if (!lval_eq(x->cell[i], y->cell[i])) return 0;
            }

            // otherwise 
            return 1;
        
        break;
    }
    // if our switch statement isn't exshaustive
    return 0;
}

lval* builtin_eq(lenv* e, lval* a) {
    LASSERT_ARGS_NUM("=", a, 2);
    int r = lval_eq(a->cell[0], a->cell[1]);
    lval_del(a);
    return lval_num(r);
}

lval* builtin_ord(lenv* e, lval* a, char* op) {
    LASSERT_ARGS_NUM(op, a, 2);
    LASSERT_ARGS_TYPE(op, a, 0, LVAL_NUM);
    LASSERT_ARGS_TYPE(op, a, 1, LVAL_NUM);

    int r;

    if (strcmp(op, ">") == 0) {
        r = (a->cell[0]->num > a->cell[1]->num);
    }

    if (strcmp(op, "<") == 0) {
        r = (a->cell[0]->num < a->cell[1]->num);
    }

    if (strcmp(op, ">=") == 0) {
        r = (a->cell[0]->num >= a->cell[1]->num);
    }

    if (strcmp(op, "<=") == 0) {
        r = (a->cell[0]->num <= a->cell[1]->num);
    }
    if (strcmp(op, "=") == 0) {
        r =  lval_eq(a->cell[0], a->cell[1]);
    }

    lval_del(a);
    return lval_num(r);
}

lval* builtin_gt(lenv* e, lval* a) {
    return builtin_ord(e, a, ">");
}

lval* builtin_ge(lenv* e, lval* a) {
    return builtin_ord(e, a, ">=");
}

lval* builtin_lt(lenv* e, lval* a) {
    return builtin_ord(e, a, "<");
}

lval* builtin_le(lenv* e, lval* a) {
    return builtin_ord(e, a, "<=");
}

lval* builtin_and(lenv* e, lval* a) {
    LASSERT_ARGS_NUM("and", a, 2);
    LASSERT_ARGS_TYPE("and", a, 0, LVAL_NUM);
    LASSERT_ARGS_TYPE("and", a, 1, LVAL_NUM);

    lval* x = lval_pop(a, 0);
    lval* y = lval_pop(a, 0);

    int result = x->num && y->num;

    free(x); free(y); free(a);
    return lval_num(result);
}

lval* builtin_or(lenv* e, lval* a) {
    LASSERT_ARGS_NUM("or", a, 2);
    LASSERT_ARGS_TYPE("or", a, 0, LVAL_NUM);
    LASSERT_ARGS_TYPE("or", a, 1, LVAL_NUM);

    lval* x = lval_pop(a, 0);
    lval* y = lval_pop(a, 0);

    int result = x->num || y->num;

    free(x); free(y); free(a);
    return lval_num(result);
}

lval* builtin_not(lenv* e, lval* a) {
    LASSERT_ARGS_NUM("not", a, 1);
    LASSERT_ARGS_TYPE("or", a, 0, LVAL_NUM);

    lval* x = lval_pop(a, 0);

    int result = !(x->num);

    free(x); free(a);
    return lval_num(result);
}

lval* builtin_if(lenv* e, lval* a) {
    LASSERT_ARGS_NUM("if", a, 3);
    LASSERT_ARGS_TYPE("if", a, 0, LVAL_NUM);
    LASSERT_ARGS_TYPE("if", a, 1, LVAL_QEXPR);
    LASSERT_ARGS_TYPE("if", a, 2, LVAL_QEXPR);

    lval* x;
    // evaluate the two expressions
    a->cell[1]->type = LVAL_SEXPR;
    a->cell[2]->type = LVAL_SEXPR;

    if (a->cell[0]->num) {
        x = lval_eval(e, lval_pop(a, 1));
    } else {
        x = lval_eval(e, lval_pop(a, 2));
    }

    lval_del(a);
    return x;
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
    v->type = LVAL_SEXPR;
    // delete all the other elements
    while (v->count > 1) lval_del(lval_pop(v, 1));
    return lval_eval(e, v);
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

// used for binding values to symbols
lval* builtin_var(lenv* e, lval* a, char* func) {
    LASSERT_ARGS_TYPE(func, a, 0, LVAL_QEXPR);

    // the first symbol should contain the argument symbols
    lval* syms = a->cell[0];
    for (int i = 0; i < syms->count; i++) {
        LASSERT(a, (syms->cell[i]->type == LVAL_SYM),
            "Function '%s' cannot define non-symbol | Got %s, expected %s",
            ltype_name(syms->cell[i]->type), ltype_name(LVAL_SYM));
    }

    LASSERT(a, (syms->count == a->count - 1),
        "Function '%s' passed too many arguments for symbols | got %i, expected %i",
        func, syms->count, a->count - 1);

    for (int i = 0; i < syms->count; i++) {
        // if `def` define it globally. if `ler` define it locally
        if (strcmp(func, "def") == 0) {
            lenv_def(e, syms->cell[i], a->cell[i + 1]);
        } 
        if (strcmp(func, "let") == 0) {
            lenv_put(e, syms->cell[i], a->cell[i + 1]);
        }
    }

    lval_del(a);
    return lval_sexpr();
}

// bind a value / function to a symbol. 
// takes 1+ symbols in a qexpr, and then a list for each symbol to bind
// uses lenv_put (basically a bootleg hashmap) to mutate the environment
lval* builtin_def(lenv* e, lval* a) {
    return builtin_var(e, a, "def");
}

// like builtin_def() but locally scoped
lval* builtin_let(lenv* e, lval* a) {
    return builtin_var(e, a, "let");
}

// create a new annonymous function
lval* builtin_lambda(lenv* e, lval* a) {
    // check for any potential user errors
    LASSERT_ARGS_NUM("\\", a, 2);
    LASSERT_ARGS_TYPE("\\", a, 0, LVAL_QEXPR);
    LASSERT_ARGS_TYPE("\\", a, 1, LVAL_QEXPR);
    for (int i = 0; i < a->cell[0]->count; i++) {
        LASSERT(a, (a->cell[0]->cell[i]->type == LVAL_SYM), 
            "Cannot define non-symbol | Got %s, expected %s",
            ltype_name(a->cell[0]->cell[i]->type), ltype_name(LVAL_SYM));
    }

    // popping out the first two args which we will give to lval_lambda
    lval* formals = lval_pop(a, 0);
    lval* body = lval_pop(a, 0);
    lval_del(a);

    return lval_lambda(formals, body);
}

lval* builtin_print(lenv* e, lval* a) {
    LASSERT_ARGS_NUM("print", a, 1);
    LASSERT_ARGS_TYPE("print", a, 0, LVAL_STR);

    puts(a->cell[0]->str);

    putchar('\n');
    lval_del(a);
    return lval_sexpr();
}

lval* builtin_exit(lenv* e, lval* a) {
    LASSERT_ARGS_NUM("exit", a, 1);
    LASSERT_ARGS_TYPE("exit", a, 0, LVAL_NUM);

    printf("\033[91mProgram ending...\033[0m\n");
    exit(a->cell[0]->num);
    return lval_sexpr();
}

lval* builtin_error(lenv* e, lval* a) {
    LASSERT_ARGS_NUM("error", a, 1);
    LASSERT_ARGS_TYPE("error", a, 0, LVAL_STR);

    lval* err = lval_err(a->cell[0]->str);

    lval_del(a);
    return err;
}

lval* builtin_atoi(lenv* e, lval* a) {
    LASSERT_ARGS_NUM("atoi", a, 1);
    LASSERT_ARGS_TYPE("atoi", a, 0, LVAL_STR);

    lval* n = lval_pop(a, 0);
    lval_del(a);

    errno = 0;
    long x = strtol(n->str, NULL, 10);
    return errno != ERANGE ? 
        lval_num(x) : lval_err("not a number");
}

lval* builtin_itoa(lenv* e, lval* a) {
    LASSERT_ARGS_NUM("itoa", a, 1);
    LASSERT_ARGS_TYPE("itoa", a, 0, LVAL_NUM);

    lval* n = lval_pop(a, 0);
    lval_del(a);

    // calculating how big the str needs to be
    // number of digits (log10 rounded up) + 1 for 10x + 1 for null terminating char 
    // * sizeof char
    size_t size = (int)(((ceil(log10(n->num))) + 1) + 1) * sizeof(char);

    char* str = malloc(size);
    snprintf(str, size, "%lu", n->num);
    lval* new = lval_str(str);
    free(str); free(n);
    return new;
}

lval* builtin_strtoascii(lenv* e, lval* a) {
    LASSERT_ARGS_NUM("strtoascii", a, 1);
    LASSERT_ARGS_TYPE("strtoascii", a, 0, LVAL_STR);
    LASSERT(a, (strlen(a->cell[0]->str) == 1), 
        "'strtoascii' function string takes one char in string");

    lval* x = lval_pop(a, 0);
    int number = (int)x->str[0];
    lval_del(x);
    return lval_num(number);
}

lval* builtin_asciitostr(lenv* e, lval* a) {
    LASSERT_ARGS_NUM("asciitostr", a, 1);
    LASSERT_ARGS_TYPE("asciitostr", a, 0, LVAL_NUM);
    

    lval* x = lval_pop(a, 0);
    char character = (char)x->num;
    lval_del(x);
    char str[2];
    str[0] = character;
    str[1] = '\0';
    return lval_str(str);
}

lval* builtin_input_num(lenv* e, lval* a) {
    LASSERT_ARGS_NUM("input-num", a, 1);
    LASSERT_ARGS_TYPE("input-num", a, 0, LVAL_FUN);

    long num;
    if (scanf("%li", &num) != 1) {
        return lval_err("invalid input: not a number");
    }

    
    return lval_call(e, a->cell[0], lval_add(lval_sexpr(), lval_num(num)));
}

// takes one argument: the max
lval* builtin_random_number(lenv* e, lval* a) {
    LASSERT_ARGS_NUM("random-number", a, 1);
    LASSERT_ARGS_TYPE("random-number", a, 0, LVAL_NUM);

    srand(time(NULL));
    long r = (long) rand() % a->cell[0]->num;
    lval_del(a);
    return lval_num(r);
}