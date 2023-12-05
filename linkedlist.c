#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

typedef struct list {
    int val;
    void* next;
} list;

list* construct(list* prevlist, int value);
void print_list(list* arglist);
list* from_array(int* arr, size_t length);
list* from_args(int length, ...);

list* construct(list* prevlist, int value) {
    list* newlist = malloc(sizeof(list));
    newlist->val = value;
    newlist->next = prevlist;
    return newlist;
}

void print_list(list* arglist) {
    list* cursorptr = arglist;
    do {
        printf("%d, ", cursorptr->val);
        cursorptr = cursorptr->next;
    } while (cursorptr->next != NULL);
    
    printf("\n");
    free(cursorptr);
}

list* from_array(int* arr, size_t length) {
    list* newlist = construct(NULL, arr[length]);
    for (int i = length - 1; i >= 0 ; i--) 
        newlist = construct(newlist, arr[i]);

    return newlist;
}

list* from_args(int length, ...) {
    va_list ap;
    va_start(ap, length);

    int* arr = malloc(sizeof(int) * length);
    for (int i = 0; i < length; i++) 
        arr[i] = va_arg(ap, int);

    va_end(ap);

    list* newlist = from_array(arr, length);
    free(arr);
    return newlist;
}

int main(int argc, char **argv) {
    size_t size = sizeof(int) * (argc - 1);
    int* arr = malloc(size);   
    
    for (int i = 1; i < argc; i++) 
        arr[i - 1] = atoi(argv[i]);

    list* mylist = from_array(arr, argc - 1);
    print_list(mylist);
    free(mylist);

    print_list(from_args(7, 1, 2, 3, 4, 5, 6, 7));
}