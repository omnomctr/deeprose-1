#ifndef LENV_HEADER
#define LENV_HEADER
#include "lval.h"

lenv* lenv_new(void);
void lenv_del(lenv* e);
lval* lenv_get(lenv* e, lval* key);
void lenv_put(lenv* e, lval* key, lval* value);
lenv* lenv_copy(lenv* e);
void lenv_def(lenv* e, lval* key, lval* value);

#endif