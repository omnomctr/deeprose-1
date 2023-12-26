#include <stdio.h>
#include <stdlib.h>
#include <editline/readline.h>
#include <string.h>
#include "mpc.h"
#include "lval.h"
#include "builtin.h"

mpc_parser_t* Deeprose;

// need to have this here because it depends on mpc parser token
lval* builtin_load(lenv* e, lval* a) {
    LASSERT_ARGS_NUM("load", a, 1);
    LASSERT_ARGS_TYPE("load", a, 0, LVAL_STR);

    // parse file given by string name 
    mpc_result_t result;
    if (mpc_parse_contents(a->cell[0]->str, Deeprose, &result)) {
        // read contents
        lval* expr = lval_read(result.output);
        mpc_ast_delete(result.output);

        while (expr->count) {
            lval* x = lval_eval(e, lval_pop(expr, 0));
            // if error print it
            if (x->type == LVAL_ERR) { lval_print(x); }
            lval_del(x);
        }

        lval_del(expr);
        lval_del(a);

        return lval_sexpr();
    } else {
        // get parse error as string 
        char* err_msg = mpc_err_string(result.error);
        mpc_err_delete(result.error);

        // create error message to return
        lval* err = lval_err("Could not load library %s", err_msg);
        free(err_msg);
        lval_del(a);

        return err;

    }
}


int main(int argc, char **argv) {
    // mpc parser config stuff 
    mpc_parser_t* Number   = mpc_new("number");
    mpc_parser_t* Symbol   = mpc_new("symbol");
    mpc_parser_t* Sexpr    = mpc_new("sexpr");
    // q exprs are like quoted s expr '() in any other lisp they aren't evaluated
    mpc_parser_t* Qexpr    = mpc_new("qexpr");
    mpc_parser_t* String   = mpc_new("string");
    mpc_parser_t* Comment  = mpc_new("comment");
    mpc_parser_t* Expr     = mpc_new("expr");
    Deeprose    = mpc_new("deeprose");
 
    mpca_lang(MPCA_LANG_DEFAULT, 
    "                                       \
    number  : /-?[0-9]+/ ;                    \
    symbol  : /[a-zA-Z0-9_+\\-*\\/\\\\=<>!&?\\^\\\%]+/ ; \
    sexpr   : '(' <expr>* ')' ;               \
    qexpr   : '{' <expr>* '}' ;                \
    string  : /\"(\\\\.|[^\"])*\"/ ;  \
    comment : /;[^\\r\\n]*/ ;   \
    expr    : <number> | <symbol> | <sexpr> | <qexpr> | <string> | <comment> ; \
    deeprose   : /^/ <expr>* /$/ ;               \
    ",
        Number, Symbol, Sexpr, Qexpr, String, Comment, Expr, Deeprose);
    
    lenv* e = lenv_new();
    lenv_add_builtins(e);

    // creating a path str for the prelude 
    {
        // 1024 should be enough
        char path[1024];
        
        if (!getenv("DRLIBPATH")) {
            puts("undefined define $DRLIBPATH");
            exit(EXIT_FAILURE);
        }

        strncpy(path, getenv("DRLIBPATH"), 1024);
        strncat(path, "/stdlib.pbl", 1024 - strlen(path));

        // load file in path
        builtin_load(e, 
            lval_add(lval_sexpr(), lval_str(path)));
    }

    

    if (argc >= 2) {
        for (int i = 1; i < argc; i++) {
            // create a lval with the argument as the str 
            lval* arg = lval_add(lval_sexpr(), lval_str(argv[i]));
            lval* x = builtin_load(e, arg);
            // print out error if there is any
            if (x->type == LVAL_ERR) { lval_println(x); }
            lval_del(x);
        }
    } 
    
    puts("Deeprose version 0.1.0\n");
    puts("press C-c to exit\n");

    while (1) {
        // purple ish blue colour
        char* input = readline("\033[34mdeeprose =>\033[0m ");
        add_history(input);  

        mpc_result_t r;
        if (mpc_parse("<stdin>", input, Deeprose, &r)) {
            lval* val = lval_eval(e, lval_read(r.output));
            lval_println(val);
            mpc_ast_delete(r.output);
        } else {
            mpc_err_print(r.error);
            mpc_err_print(r.error);
        }

        free(input);
    }

    // free up parser heap memory stuff
    mpc_cleanup(5, Number, Symbol, Sexpr, Qexpr, String, Comment, Expr, Deeprose);
    lenv_del(e);
    return 0;
}