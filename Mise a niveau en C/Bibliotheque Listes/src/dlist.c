#include <dlist.h>
#include <error.h>
#include <stdlib.h>
#include <string.h>


static DListnode *new_node(DList *list, const void *element);
static dlist_status dlist_node_is_in(DList *list, const DListnode *node);


static DListnode *split_middle(DListnode *head);
static DListnode *merge_sorted(DListnode *a, DListnode *b, dlist_cmp_fn cmp);
static DListnode *msort_rec(DListnode *head, dlist_cmp_fn cmp);

static DListnode *new_node(DList *list, const void *element){
    if (list==NULL) return NULL;
    DListnode *node;
    node = malloc(sizeof*node);
    if (!node)
    {
        error_sys(__func__, "malloc");
        return NULL;
    }
    if(list->copy) node->data = list->copy(element);
    else    node->data = (void *)element;

    if (element && list->copy && !node->data)
    {
        free(node);
        return NULL;
    }
    node->next = node->prev =NULL;
    return node;
}

static dlist_status dlist_node_is_in(DList *list, const DListnode *node){
    if(!list || !node) return DLIST_INVALID_ARG;
    for(DListnode *e = list->head; e;e = e->next){
        if(e==node) return DLIST_OK;
    }
    return DLIST_NOT_FOUND;
}

DList *dlist_create(dlist_copy_fn copy, dlist_free_fn free_fn, dlist_cmp_fn cmp, dlist_print_fn print){
    DList *new_list;
    new_list = calloc(1, sizeof*new_list);
    if (!new_list)
    {
        error_sys(__func__, "calloc");
        return NULL;
    }
    new_list->head = new_list->tail= NULL;
    new_list->size = 0;

    new_list->copy = copy;
    new_list->free_fn = free_fn;
    new_list->cmp = cmp;
    new_list->print = print;

    return new_list;
}

void dlist_clear(DList *list){
    if(list==NULL) return;
    for (DListnode *e = list->head; e;)
    {
        DListnode *tmp = e->next;
        if (list->free_fn!=NULL && e->data!=NULL)
            list->free_fn(e->data);
        free(e);
        e = tmp;
    }
    list->head = list->tail= NULL;
    list->size = 0; 
}

void dlist_destroy(DList *list){
    if(!list) return;
    dlist_clear(list);
    free(list);
}

dlist_status dlist_push_front(DList *list,const void *element){
    if (!list || !element)return DLIST_INVALID_ARG;
    DListnode *node = new_node(list, element);
    if (!node) return DLIST_NOMEM;
    if (!list->head){
        list->tail = list->head = node;
        list->size++;
        return DLIST_OK;
    }
    
    node->next = list->head;
    list->head->prev = node;
    list->head = node;
    list->size++;
    return DLIST_OK;
}

dlist_status dlist_push_back(DList *list,const void *element){
    if (list==NULL || !element) return DLIST_INVALID_ARG;

    DListnode *node = new_node(list, element);
    if (!node) return DLIST_NOMEM;
    if (!list->head){
        list->tail = list->head = node;
        list->size++;
        return DLIST_OK;
    }
    node->prev = list->tail;
    list->tail->next = node;
    list->tail = node;
    list->size++;
    return DLIST_OK;
}

dlist_status dlist_insert_before(DList *list,DListnode *posi, const void *element){
    if (!list || !element) return DLIST_INVALID_ARG;

    if(!posi) return dlist_push_back(list, element);
    DListnode *node = new_node(list, element);
    if (!node) return DLIST_NOMEM;
    node->next = posi;
    node->prev = posi->prev;
    if(posi->prev) posi->prev->next = node;
    else list->head = node;
    posi->prev = node;
    list->size++;
    return DLIST_OK;
}

dlist_status dlist_insert_after(DList *list,DListnode *posi, const void *element){
    if (!list || !element) return DLIST_INVALID_ARG;
    if(!posi) return dlist_push_front(list, element);
    DListnode *node = new_node(list, element);
    if (!node) return DLIST_NOMEM;
    node->next = posi->next;
    node->prev = posi;
    if(posi->next) posi->next->prev = node;
    else list->tail = node;
    posi->next = node;
    list->size++;
    return DLIST_OK;
}

DListnode *dlist_begin(const DList *list){
    return list? list->head: NULL;
}

DListnode *dlist_rbegin(const DList *list){
    return list? list->tail: NULL;
}

DListnode *dlist_next(const  DListnode *node){
    return node? node->next: NULL;
}

DListnode *dlist_prev(const  DListnode *node){
    return node? node->prev: NULL;
}

void *dlist_data(const DListnode *node){
    return node? node->data:NULL;
}

dlist_status dlist_pop_front(DList *list){
    if(!list) return DLIST_OK;
    DListnode *node = list->head;

    if(!node) return DLIST_INVALID_ARG;
    if (node->next)
    {
        node->next->prev = NULL;
        list->head = node->next;
    }else
    {
        list->head = list->tail = NULL;
    }
    if (list->free_fn && node->data) list->free_fn(node->data);
    free(node);
    list->size--;
    return DLIST_OK;
 }


 dlist_status dlist_pop_back(DList *list){
    if(!list) return DLIST_OK;

    DListnode *node = list->tail;

    if(!node) return DLIST_INVALID_ARG;

    if (node->prev)
    {
        node->prev->next = NULL;
        list->tail = node->prev;
    }else
    {
        list->head = list->tail =NULL;
    }
    if(list->free_fn && node->data) list->free_fn(node->data);
    free(node);
    list->size--;
    return DLIST_OK;  
 }

dlist_status dlist_erase(DList *list, DListnode *node){
    if(!list) return DLIST_OK;
    if(!node) return DLIST_INVALID_ARG;

    dlist_status verif = dlist_node_is_in(list, node);
    if(verif != DLIST_OK) return verif;
    if(!node->next) return dlist_pop_back(list);
    if(!node->prev) return dlist_pop_front(list);
    node->next->prev = node->prev;
    node->prev->next = node->next;
    if (list->free_fn && node->data)
    {
        list->free_fn(node->data);
    }
    free(node);
    list->size--;
    return DLIST_OK;
}


void *dlist_extract(DList *list, DListnode *node){
    if(!list || !node) return NULL;

    if(dlist_node_is_in(list, node)!=DLIST_OK) return NULL;

    
    void *data = node->data;

    if(node->prev) node->prev->next = node->next;
    else list->head = node->next;
    if(node->next) node->next->prev = node->prev;
    else list->tail = node->prev;

    node->prev = node->next = NULL;
    node->data = NULL;
    list->size--;
    free(node);
    return data;
}


DListnode *dlist_find_first(const DList *list,const void *key){
    if(!list || !key || !list->cmp) return NULL;

    for (const DListnode *e = list->head; e; e=e->next){
        if (e->data && list->cmp(e->data, key)==0)
        {
            return e->data;
        }
    }
    return NULL;
}


DListnode *dlist_find_if(const DList *list, int (*pred)(const void *elem, void *ctx), void *ctx){
    if(!list || !pred) return NULL;

    for(DListnode *e = list->head; e; e=e->next){
        if(e->data)
            if(pred(e->data, ctx))
                return e;
    }
    return NULL;
}


size_t dlist_count(const DList *list, const void *key){
    if(!list || !list->cmp || !key) return 0;

    size_t count = 0;
    for(const DListnode *e = list->head; e; e= e->next){
        if(e->data && !list->cmp(e->data, key))
            count++;
    }
    return count;
}


void dlist_reverse(DList *list){
    if (!list || !list->head || !list->head->next) return;

    
    for(DListnode *e = list->head; e; e=e->prev){
        DListnode *tmp=e->next;
        e->next = e->prev;
        e->prev = tmp;
    }

    DListnode *temp = list->head;
    list->head = list->tail;
    list->tail = temp;
}

/*Bon mais sa complexite est O(n)*/
// void dlist_concat(DList *dst, DList *src){
//     for(DListnode *e=src->head; e; e = e->next){
//         dlist_status verif = dlist_push_back(dst, e->data);
//         if(verif!=DLIST_OK)
//             return;
//     }
// }

dlist_status dlist_concat(DList *dst, DList *src){
    if(!dst || !src) return DLIST_INVALID_ARG;

    if(dst==src) return DLIST_INVALID_ARG;

    if (dst->free_fn != src->free_fn || dst->copy != src->copy || dst->cmp != src->cmp || dst->print != src->print)
        return DLIST_INVALID_ARG;

    if (src->size == 0) return DLIST_OK;

    if(dst->size == 0){
        dst->head = src->head;
        dst->tail = src->tail;
    }else{
        dst->tail->next = src->head;
        src->head->prev = dst->tail;
        dst->tail = src->tail;
    }
    dst->size += src->size;

    src->head = src->tail = NULL;
    src->size=0;
    return DLIST_OK;
}

dlist_status dlist_splice_before(DList *dst, DListnode *pos, DList *src){
    if(!dst || !src) return DLIST_INVALID_ARG;

    if(dst==src) return DLIST_INVALID_ARG;

    if (dst->free_fn != src->free_fn || dst->copy != src->copy || dst->cmp     != src->cmp     || dst->print != src->print)
        return DLIST_INVALID_ARG;
    
    if(!pos) return dlist_concat(dst,src);

    if(!src->size) return DLIST_OK;


    dlist_status verif = dlist_node_is_in(dst, pos);
    if(verif!=DLIST_OK) return verif;

    if (dlist_node_is_in(src, pos) == DLIST_OK) return DLIST_INVALID_ARG;

    if (!pos->prev)
    {
        dst->head = src->head;
        src->head->prev = NULL;
    }else{
        pos->prev->next = src->head;
        src->head->prev = pos->prev;
    }

    src->tail->next = pos;
    pos->prev = src->tail;

    dst->size+=src->size;

    src->head = src->tail = NULL;
    src->size=0;
    return DLIST_OK;
}

DList *dlist_split_after(DList *src, DListnode *pos){
    if(!src) return NULL;
    DList *out_list = dlist_create(src->copy, src->free_fn, src->cmp, src->print);
    if(!out_list) return NULL;

    if(src->size == 0) return out_list;
    if(!pos){
        out_list->head = src->head;
        out_list->tail = src->tail;
        out_list->size = src->size;
        src->head = src->tail =NULL;
        src->size = 0;
        return out_list;
    }
    dlist_status verif = dlist_node_is_in(src, pos);
    if(verif!=DLIST_OK){
        dlist_destroy(out_list);
        return NULL;
    }

    if(!pos->next) return out_list;

    out_list->head = pos->next;
    out_list->tail = src->tail;

    pos->next = NULL;
    pos->next->prev = NULL;
    src->tail = pos;
    out_list->size = 0;
    for(const DListnode *e = out_list->head; e; e=e->next){
        out_list->size++;
    }
    src->size -= out_list->size;
    return out_list;
}


dlist_status dlist_unique(DList *list){
    if(!list || !list->cmp) return DLIST_INVALID_ARG;
    if (!list->head || !list->head->next) return DLIST_OK;

    DListnode *cur = list->head;
    while (cur && cur->next) {
        DListnode *nxt = cur->next;
        if (list->cmp(cur->data, nxt->data) == 0) {
            dlist_status st = dlist_erase(list, nxt);
            if (st != DLIST_OK) return st;
        } else {
            cur = nxt;
        }
    }
    return DLIST_OK;
}

dlist_status dlist_remove_if(DList *list, int (*pred)(const void *elem, void *ctx), void *ctx){
    if(!list || !pred) return DLIST_INVALID_ARG;

    for (DListnode *e = list->head; e;)
    {
        DListnode *next = e->next;
        if(pred(e->data, ctx)){
            dlist_status verif = dlist_erase(list, e);
            if(verif!=DLIST_OK) return verif;
        }
        e=next;
    }
    
    return DLIST_OK;
}

dlist_status dlist_sort(DList *list){
    if (!list || !list->cmp) return DLIST_INVALID_ARG;
    if (!list->head || !list->head->next) return DLIST_OK;

    list->head->prev = NULL;

    DListnode *new_head = msort_rec(list->head, list->cmp);

    list->head = new_head;

    DListnode *tail = new_head;
    while (tail && tail->next) tail = tail->next;

    if (list->head) list->head->prev = NULL;
    if (tail)       tail->next      = NULL;
    list->tail = tail;

    return DLIST_OK;
}

static DListnode *split_middle(DListnode *head){
    if(!head || !head->next) return NULL;

    DListnode *slow = head;
    DListnode *fast = head;
    DListnode *prev_slow = NULL;

    while (fast && fast->next)
    {
        prev_slow = slow;
        slow = slow->next;
        fast = fast->next->next;
    }

    prev_slow->next = NULL;
    if(slow) slow->prev = NULL;
    return slow;
}

static DListnode *msort_rec(DListnode *head, dlist_cmp_fn cmp){
    if(!head || !head->next) return head;

    DListnode *head_right = split_middle(head);

    DListnode *left = msort_rec(head, cmp);
    DListnode *right = msort_rec(head_right, cmp);

    return merge_sorted(left, right,cmp);
}

static DListnode *merge_sorted(DListnode *a, DListnode *b, dlist_cmp_fn cmp){
    if(!a) return b;
    if(!b) return a;

    DListnode *res_head = NULL;
    DListnode *tail = NULL;

    while (a && b)
    {
        DListnode *pick;
        if (cmp(a->data, b->data) <= 0)
        {
            pick = a;
            a = a->next;
        }else
        {
            pick = b;;
            b = b->next;
        }
        if (!res_head)
        {
            res_head = tail = pick;
            tail->prev = NULL;
            tail->next = NULL;
        }else
        {
            tail->next = pick;
            pick->prev = tail;
            tail = pick;
            tail->next = NULL;
        }   
    }
    DListnode *rest = a ? a:b;

        if (rest)
        {
            if (!res_head)
            {
                res_head = rest;
                res_head->prev = NULL;
            }else
            {
                tail->next = rest;
                rest->prev = tail;
            }
        }
    return res_head;
}

DList *dlist_map(const DList *in, void *(*map_fn)(const void *elem, void *ctx), void *ctx, dlist_free_fn out_free) {
    if (!in || !map_fn) return NULL;
    DList *out = dlist_create(NULL, out_free, in->cmp, in->print);
    if (!out) return NULL;
    for (DListnode *p = in->head; p; p = p->next) {
        void *y = map_fn(p->data, ctx);
        if (dlist_push_back(out, y) != DLIST_OK) { 
            dlist_destroy(out);
            return NULL;
        }
    }
    return out;
}


DList *dlist_filter(const DList *in, int (*pred)(const void *elem, void *ctx), void *ctx) {
    if (!in || !pred) return NULL;
    DList *out = dlist_create(in->copy, in->free_fn, in->cmp, in->print);
    if (!out) return NULL;
    for (DListnode *p = in->head; p; p = p->next) {
        if (pred(p->data, ctx)) {
            if (dlist_push_back(out, p->data) != DLIST_OK) {
                dlist_destroy(out);
                return NULL;
            }
        }
    }
    return out;
}


DList *dlist_clone(const DList *in) {
    if (!in) return NULL;
    DList *out = dlist_create(in->copy, in->free_fn, in->cmp, in->print);
    if (!out) return NULL;
    for (DListnode *p = in->head; p; p = p->next)
    if (dlist_push_back(out, p->data) != DLIST_OK) {
        dlist_destroy(out);
        return NULL;
    }
return out;
}


void **dlist_to_array(const DList *list, size_t *out_len) {
    if (!list) return NULL;
    if (out_len) *out_len = list->size;
    void **arr = (void**)calloc(list->size, sizeof *arr);
    if (!arr) return NULL;
    size_t i = 0;
    for (DListnode *p = list->head; p; p = p->next) arr[i++] = p->data;
    return arr; // pointeurs *tels quels*
}


DList *dlist_from_array(void *const *arr, size_t len, dlist_copy_fn copy, dlist_free_fn free_fn, dlist_cmp_fn cmp, dlist_print_fn print) {
    DList *out = dlist_create(copy, free_fn, cmp, print);
    if (!out) return NULL;
    for (size_t i = 0; i < len; i++){
        if (dlist_push_back(out, arr[i]) != DLIST_OK) {
            dlist_destroy(out);
            return NULL;
        }
    }
    return out;
}




