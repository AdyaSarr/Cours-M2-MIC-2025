#include "list.h"
#include <string.h>
#include <stdlib.h>
#include "error.h"

static elt_t *copy_elt(const elt_t *);
static void print_elt(const elt_t *, FILE *);

int list_insert(list_t l, const elt_t * element, int flg){
    elt_t *n;
    if (flg)
    {
        n=copy_elt(element);
        if (!n)
        {
            return -1;
        }
        
    }else
    {
        n = element;
    }
    
    if(!n){
        return -1;
    }
    n->next = l->first;
    l->first = n;
    if(!l->last){
        l->last = n;
    }
    l->len++;
    return 0;
}

static elt_t *copy_elt(const elt_t * element){
    elt_t *ret;
    size_t dlen;
    ret = malloc(sizeof(elt_t));
    if (!ret)
        return NULL;
    ret->type = element->type;
    ret->next = NULL;
    switch (element->type)
    {
    case DATA_INT:
        /* code */
        dlen = sizeof(int);
        break;
    case DATA_STR:
        dlen = strlen(element->data);
        break;
    default:
        break;
    }
    ret->data =  malloc(dlen);
    if (!ret->data)
    {
        free(ret);
        return NULL;
    }
    memcpy(ret->data, element->data, dlen);
    return ret;
}
void list_print(const list_t l, FILE *os){
    fprintf(os, "[");
    for (elt_t *element = l->first; element; element = element->next)
    {
        print_elt(element, os);
        if (element->next)
        {
            /* code */
            fprintf(os, ", ");
        }
    }
    fprintf(os, "]");
}

static void print_elt(const elt_t * element, FILE *os){
    switch (element->type)
    {
    case DATA_INT:
        /* code */
        fprintf(os, "%d", *(int *)element->data);
        break;
    case DATA_STR:/*Le format %s attend un pointeur*/
        fprintf(os, "%s", (char *)element->data);
        break;
    }
}

int list_insert_int(list_t l, int i){
    elt_t *n;
    n=malloc(sizeof(*n));
    if (!n)
    {
        error_sys(__func__, "malloc");
        return -1;
    }
    n->type = DATA_INT;
    n->data = malloc(sizeof(int));
    if (!n->data)
    {
        free(n);
        return -1;
    }
    memcmp(n->data, &i, sizeof(int));
    return list_insert(l, n, 0);
}

list_insert_str(list_t l,const char * chaine){
    elt_t *n;
    n=malloc(sizeof(*n));
    if (!n)
    {
        return -1;
    }
    n->type = DATA_STR;
    n->data = malloc(strlen(chaine) + 1);
    if (!n->data)
    {
        free(n);
        return -1;
    }
    memcmp(n->data, &chaine, strlen(chaine) + 1);
    return list_insert(l, n, 0);

}

int list_concat(const list_t l1, const list_t l2, list_t l){
    
}




int list_insert_list(list_t, list_t){
    
}