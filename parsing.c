#include <stdio.h>
#include <stdlib.h>
#include <editline/readline.h>
#include <string.h>
#include "mpc.h"
#include "lval.h"
#include "builtin.h"

int main(void) {
    // mpc parser config stuff 
    mpc_parser_t* Number   = mpc_new("number");
    mpc_parser_t* Symbol   = mpc_new("symbol");
    mpc_parser_t* Sexpr    = mpc_new("sexpr");
    // q exprs are like quoted s expr '() in any other lisp they aren't evaluated
    mpc_parser_t* Qexpr    = mpc_new("qexpr");
    mpc_parser_t* Expr     = mpc_new("expr");
    mpc_parser_t* Pablo    = mpc_new("pablo");
 
    mpca_lang(MPCA_LANG_DEFAULT, 
    "                                       \
    number : /-?[0-9]+/ ;                    \
    symbol : /[a-zA-Z0-9_+\\-*\\/\\\\=<>!&?\\^\\\%]+/ ; \
    sexpr  : '(' <expr>* ')' ;               \
    qexpr  : '{' <expr>* '}' ;                \
    expr   : <number> | <symbol> | <sexpr> | <qexpr> ; \
    pablo  : /^/ <expr>* /$/ ;               \
    ",
        Number, Symbol, Sexpr, Qexpr, Expr, Pablo);
    puts("Pablo version 0.0.0.0.1\n");
    puts("press C-c to exit\n");
    
    lenv* e = lenv_new();
    lenv_add_builtins(e);

    while (1) {
        char* input = readline("pablo => ");
        add_history(input);  

        mpc_result_t r;
        if (mpc_parse("<stdin>", input, Pablo, &r)) {
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
    mpc_cleanup(5, Number, Symbol, Sexpr, Qexpr, Expr, Pablo);
    lenv_del(e);
    return 0;
}