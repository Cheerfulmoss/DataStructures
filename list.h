// Created by alex on 2/09/2024.
//
// This implementation of a generic list was inspired by the work of TheBrokenPipe.
// Original source: https://github.com/TheBrokenPipe/CListTemplate/blob/main/list.h

#ifndef DATASTRUCTURES_LIST_H
#define DATASTRUCTURES_LIST_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stddef.h>
#include <stdint.h>


typedef intptr_t ssize_t;
typedef int (*compare_func)(const void*, const void*);

typedef enum {
    MEMORY_ALLOCATION_ERR = 1,
    INDEX_OUT_OF_RANGE = 2,
    UNINITIALISED_ARRAY = 3,
} ListErrors;

#define IMPORT_LIST(type, container)                                           \
typedef struct _##container {                                                  \
    ssize_t size;                                                              \
    ssize_t capacity;                                                          \
    type *data;                                                                \
                                                                               \
    int (*get)(struct _##container*, ssize_t, type*);                          \
    int (*set)(struct _##container*, ssize_t, type);                           \
    ssize_t (*len)(struct _##container*);                                      \
    ssize_t (*cap)(struct _##container*);                                      \
    int (*append)(struct _##container*, type);                                 \
    int (*pop)(struct _##container*, ssize_t, type*);                          \
    int (*insert)(struct _##container*, ssize_t, type);                        \
    void (*sort)(struct _##container*, ssize_t, ssize_t, compare_func);        \
    int (*foreach)(struct _##container*, int (struct _##container*, ssize_t)); \
    ssize_t (*find)(struct _##container*, type, compare_func);                 \
    int (*shuffle)(struct _##container*, ssize_t, ssize_t);                    \
    struct _##container* (*slice)(struct _##container*, ssize_t, ssize_t,      \
        ssize_t);                                                              \
    void (*reverse)(struct _##container*);                                     \
    void (*clear)(struct _##container*);                                       \
    void (*destroy)(struct _##container*);                                     \
    int (*resize)(struct _##container*);                                       \
} _##container;                                                                \
                                                                               \
typedef _##container *container;                                               \
typedef int (*foreach_func_##container)(container, ssize_t);                   \
                                                                               \
static int container##_resize(container list);                                 \
static int container##_get(container list, ssize_t index, type* result);       \
static int container##_set(container list, ssize_t index, type element);       \
static ssize_t container##_size(container list);                               \
static ssize_t container##_capacity(container list);                           \
static int container##_append(container list, type element);                   \
static int container##_pop(container list, ssize_t index, type *result);       \
static int container##_insert(container list, ssize_t index, type element);    \
static void container##_sort(container list, ssize_t left, ssize_t right,      \
                             compare_func cmp);                                \
static ssize_t container##_find(container list, type element,                  \
                                compare_func cmp);                             \
static int container##_foreach(container list, foreach_func_##container func); \
static container container##_slice(container list, ssize_t left,               \
                                    ssize_t right, ssize_t step);              \
static int container##_shuffle(container list, ssize_t left, ssize_t right);   \
static void container##_reverse(container list);                               \
static void container##_clear(container list);                                 \
static void container##_destroy(container list);                               \
                                                                               \
static ssize_t container##_size(container list)                                \
{                                                                              \
    return list->size;                                                         \
}                                                                              \
                                                                               \
static ssize_t container##_capacity(container list)                            \
{                                                                              \
    return list->capacity;                                                     \
}                                                                              \
                                                                               \
static container container##_new()                                             \
{                                                                              \
    container result = (container)calloc(1, sizeof(_##container));             \
    if (result) {                                                              \
        result->data = NULL;                                                   \
        result->capacity = 0;                                                  \
        result->size = 0;                                                      \
                                                                               \
        /* Function APIs */                                                    \
        result->get = container##_get; result->set = container##_set;          \
        result->len = container##_size; result->append = container##_append;   \
        result->pop = container##_pop; result->insert = container##_insert;    \
        result->clear = container##_clear; result->cap = container##_capacity; \
        result->destroy = container##_destroy;                                 \
        result->sort = container##_sort;                                       \
        result->foreach = container##_foreach;                                 \
        result->find = container##_find;                                       \
        result->shuffle = container##_shuffle;                                 \
        result->resize = container##_resize;                                   \
        result->reverse = container##_reverse;                                 \
        result->slice = container##_slice;                                     \
    }                                                                          \
    return result;                                                             \
}                                                                              \
                                                                               \
static int container##_get(container list, ssize_t index, type* result)        \
{                                                                              \
    if (!list) return UNINITIALISED_ARRAY;                                     \
    if (list->len(list) <= index) return INDEX_OUT_OF_RANGE;                   \
    *result = list->data[index];                                               \
    return 0;                                                                  \
}                                                                              \
                                                                               \
static int container##_set(container list, ssize_t index, type element)        \
{                                                                              \
    if (!list) return UNINITIALISED_ARRAY;                                     \
    if (list->len(list) <= index) return INDEX_OUT_OF_RANGE;                   \
    list->data[index] = element;                                               \
    return 0;                                                                  \
}                                                                              \
                                                                               \
static int container##_append(container list, type element)                    \
{                                                                              \
    int retCode;                                                               \
    if ((retCode = list->resize(list))) return retCode;                        \
    list->data[list->size++] = element;                                        \
    return retCode;                                                            \
}                                                                              \
                                                                               \
static int container##_pop(container list, ssize_t index, type *result) {      \
    if (index >= container##_size(list)) {                                     \
        return INDEX_OUT_OF_RANGE;                                             \
    }                                                                          \
    *result = list->data[index];                                               \
    memmove(&list->data[index], &list->data[index + 1],                        \
            (--(list->size) - index) * sizeof(type));                          \
    int retCode;                                                               \
    if ((retCode = list->resize(list))) return retCode;                        \
    return retCode;                                                            \
}                                                                              \
                                                                               \
static int container##_insert(container list, ssize_t index, type element)     \
{                                                                              \
    int retCode = 0;                                                           \
    if (!index && !list->len(list)) {                                          \
        retCode = list->append(list, element);                                 \
        return retCode;                                                        \
    }                                                                          \
    if (index >= list->len(list)) {                                            \
        return INDEX_OUT_OF_RANGE;                                             \
    }                                                                          \
    list->size++;                                                              \
    if ((retCode = list->resize(list))) {                                      \
        list->size--;                                                          \
        return retCode;                                                        \
    }                                                                          \
    memmove(&list->data[index + 1], &list->data[index],                        \
            (list->len(list) - index) * sizeof(type));                         \
    list->data[index] = element;                                               \
    return retCode;                                                            \
}                                                                              \
                                                                               \
static void container##_clear(container list)                                  \
{                                                                              \
    if (!list->data) return;                                                   \
    free(list->data);                                                          \
    list->size = 0;                                                            \
    list->data = NULL;                                                         \
    list->capacity = 0;                                                        \
}                                                                              \
                                                                               \
static void container##_destroy(container list)                                \
{                                                                              \
    if (!list) return;                                                         \
    list->clear(list);                                                         \
    free(list);                                                                \
}                                                                              \
                                                                               \
static void container##_sort(container list, ssize_t left, ssize_t right,      \
                             compare_func cmp)                                 \
{                                                                              \
    if (left >= right) return;                                                 \
                                                                               \
    type pivot = list->data[left + (right - left) / 2];                        \
    ssize_t i = left;                                                          \
    ssize_t j = right;                                                         \
                                                                               \
    while (i <= j) {                                                           \
        while (cmp(&list->data[i], &pivot) < 0) i++;                           \
        while (cmp(&list->data[j], &pivot) > 0) j--;                           \
                                                                               \
        if (i <= j) {                                                          \
            type tmp = list->data[i];                                          \
            list->data[i] = list->data[j];                                     \
            list->data[j] = tmp;                                               \
            i++;                                                               \
            j--;                                                               \
        }                                                                      \
    }                                                                          \
    container##_sort(list, left, j, cmp);                                      \
    container##_sort(list, i, right, cmp);                                     \
}                                                                              \
                                                                               \
static int container##_foreach(container list, foreach_func_##container func)  \
{                                                                              \
    if (!list || !list->data || !func) return UNINITIALISED_ARRAY;             \
                                                                               \
    int retCode = 0;                                                           \
    for (ssize_t i = 0; i < list->len(list); i++) {                            \
        retCode = func(list, i);                                               \
        if (retCode) return retCode;                                           \
    }                                                                          \
    return 0;                                                                  \
}                                                                              \
                                                                               \
static ssize_t container##_find(container list, type element,                  \
    compare_func cmp)                                                          \
{                                                                              \
    for (ssize_t i = 0; i < list->len(list); i++) {                            \
        if (cmp(&list->data[i], &element) == 0) return i;                      \
    }                                                                          \
    return -1;                                                                 \
}                                                                              \
                                                                               \
static int container##_shuffle(container list, ssize_t left, ssize_t right)    \
{                                                                              \
    if (list->size <= 1) return 0;                                             \
                                                                               \
    if (right <= left || left < 0 || right >= list->len(list))                 \
        return INDEX_OUT_OF_RANGE;                                             \
                                                                               \
    srand((unsigned int) time(NULL));                                          \
                                                                               \
    for (ssize_t i = right; i > left; i--) {                                   \
        ssize_t j = rand() % (i + 1);                                          \
                                                                               \
        type tmp = list->data[i];                                              \
        list->data[i] = list->data[j];                                         \
        list->data[j] = tmp;                                                   \
    }                                                                          \
    return 0;                                                                  \
}                                                                              \
                                                                               \
static void container##_reverse(container list)                                \
{                                                                              \
    for (ssize_t i = 0; i < list->len(list) / 2; i++) {                        \
        type tmp = list->data[i];                                              \
        list->data[i] = list->data[list->len(list) - i - 1];                   \
        list->data[list->len(list) - i - 1] = tmp;                             \
    }                                                                          \
}                                                                              \
                                                                               \
static container container##_slice(container list,                             \
    ssize_t left, ssize_t right, ssize_t step)                                 \
{                                                                              \
    if (left < 0 || right > list->len(list) || left > right)                   \
        return NULL;                                                           \
    if (step < 1) return NULL;                                                 \
    container new_list = container##_new();                                    \
    if (step == 1) {                                                           \
        new_list->size = right - left / step;                                  \
        new_list->resize(new_list);                                            \
        memmove(new_list->data, &list->data[left],                             \
            (right - left - 1) * sizeof(type));                                \
    } else {                                                                   \
        for (ssize_t i = left; i < right; i += step)                           \
            if(new_list->append(new_list, list->data[i])) return NULL;         \
    }                                                                          \
    return new_list;                                                           \
}                                                                              \
                                                                               \
static int container##_resize(container list)                                  \
{                                                                              \
    ssize_t new_capacity = list->capacity;                                     \
    if (list->size >= list->capacity) {                                        \
        while (new_capacity <= list->len(list)) {                              \
            new_capacity = new_capacity ? new_capacity * 2 : 1;                \
        }                                                                      \
    } else if (list->len(list) < list->capacity / 4) {                         \
        while (new_capacity > (list->size * 4)) {                              \
            new_capacity /= 2;                                                 \
        }                                                                      \
    }                                                                          \
    type *new_data = (type *) realloc(list->data, new_capacity * sizeof(type));\
    if (new_data) {                                                            \
        list->data = new_data;                                                 \
        list->capacity = new_capacity;                                         \
        return 0;                                                              \
    }                                                                          \
    return MEMORY_ALLOCATION_ERR;                                              \
}                                                                              \
                                                                               \

#define NEW(container) container##_new()

#endif //DATASTRUCTURES_LIST_H
