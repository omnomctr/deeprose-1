gcc --std=c99 \
    -Wall \
    -leditline \
    -lm \
    parsing.c mpc.c lval.c builtin.c lenv.c \
    -o deeprose

echo "done"