#if !defined(LIST_H)
#define LIST_H
#include <sys/types.h>
#include <stdio.h>

typedef enum {
    DATA_INT = 0x01,
    DATA_STR
}data_type_t;



typedef struct __elt
{
    /* data */
    data_type_t type;
    void *data;
    struct __elt *next;
}elt_t;

struct 
{
    elt_t *first;
    elt_t *last;
    size_t len;

}typedef list_t[1];
#define LIST_NULL {{NULL, NULL, NULL}}
void list_print(const list_t, FILE *);
int list_insert_int(list_t l,int );
int list_insert_str(list_t l,const char *);
int list_concat(const list_t, const list_t, list_t );

int list_insert(list_t , const elt_t *, int);
#endif // LIST_H
