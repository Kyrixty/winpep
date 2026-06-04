#ifndef RDS_LIST_H
#define RDS_LIST_H
#include "common.h"
#include <stdio.h>
#include <stdlib.h>

/* RDS List */
/*
DECL_LIST(intList, int);

int main()
{
    intList* list = intList_init();
    intList_append(list, 5);
    intList_pop(list);
    intList_get(index);
    intList_set(index, val); 
}
*/

#define LIST_EXIT_FAILURE 1
#define LIST_INITIAL_CAPACITY 8
static inline void LIST_FATAL_ERROR(const char* msg)
{
    fprintf(stderr, msg); exit(LIST_EXIT_FAILURE);
}

#define DECL_LIST(typename, type)\
typedef struct \
{\
    u64 len; \
    u64 cap; \
    type* data; \
} typename;\
typename* typename##_init()\
{\
    typename* list = malloc(sizeof(typename));\
    if (!list) LIST_FATAL_ERROR("List init : Cannot allocate list!");\
    list->data = malloc(sizeof(type) * LIST_INITIAL_CAPACITY);\
    if (!list->data) LIST_FATAL_ERROR("List init : Cannot allocate list data");\
    list->len = 0;\
    list->cap = LIST_INITIAL_CAPACITY;\
    return list;\
}\
u64 typename##_len(typename* list)\
{\
    return list->len;\
}\
u64 typename##_cap(typename* list)\
{\
    return list->cap;\
}\
_Bool typename##_is_empty(typename* list)\
{\
    return typename##_len(list) == 0;\
}\
_Bool typename##_supports_index(typename* list, u64 index)\
{\
    return (!typename##_is_empty(list) && index < typename##_len(list));\
}\
_Bool typename##_needs_resize(typename* list)\
{\
    return typename##_len(list) >= typename##_cap(list);\
}\
void typename##_resize(typename* list, u64 newCap)\
{\
    if (list->cap >= newCap) LIST_FATAL_ERROR("List resize : cannot resize to a smaller size!");\
    type* new = realloc(list->data, newCap * sizeof(type));\
    if (!new) LIST_FATAL_ERROR("List resize : cannot reallocate data buffer.");\
    list->data = new;\
    list->cap = newCap;\
}\
void typename##_append(typename* list, type val)\
{\
    if (typename##_needs_resize(list)) {\
        typename##_resize(list, typename##_cap(list) * 2);\
    }\
    list->data[list->len++] = val;\
}\
type typename##_pop(typename* list)\
{\
    if (typename##_is_empty(list)) LIST_FATAL_ERROR("List pop : cannot pop an empty list");\
    type tmp = list->data[list->len - 1];\
    list->len--;\
    return tmp;\
}\
void typename##_set(typename* list, u64 index, type val)\
{\
    if (!typename##_supports_index(list, index)) LIST_FATAL_ERROR("List set : unsupported index");\
    list->data[index] = val;\
}\
type typename##_get(typename* list, u64 index)\
{\
    if (!typename##_supports_index(list, index)) LIST_FATAL_ERROR("List get : unsupported index");\
    return list->data[index];\
}\
void typename##_free(typename* list)\
{\
    if (list->cap && list->data) free(list->data);\
    free(list);\
}\
void typename##_clear(typename* list)\
{\
    list->len = 0;\
}\
void typename##_emplace(typename* list, u64 offStart, u64 offEnd, type val)\
{\
    if (offStart >= offEnd) LIST_FATAL_ERROR("List emplace : offStart must be < offEnd (use set if you want to set individual elements)");\
    if (!typename##_supports_index(list, offEnd - 1) || !typename##_supports_index(list, offStart)) LIST_FATAL_ERROR("List emplace : offStart & (offEnd - 1) must be a supported indexes");\
    for (size_t i = offStart; i < offEnd; i++){\
        list->data[i] = val;\
    }\
}\
void typename##_fill(typename* list, u64 nToFill, type val)\
{\
    for (size_t i = 0; i < nToFill; i++){\
        typename##_append(list, val);\
    }\
}\
void typename##_reserve(typename* list, u64 rsvSize)\
{\
    if (typename##_cap(list) >= rsvSize) return;\
    typename##_resize(list, rsvSize);\
}\


#endif