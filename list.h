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

#define IMPORT_LIST(T, L)                                                      \
typedef struct _##L                                                            \
{                                                                              \
    ssize_t size;                                                              \
    ssize_t capacity;                                                          \
    T *data;                                                                   \
                                                                               \
    int (*get)(struct _##L*, ssize_t, T*);                                     \
    int (*set)(struct _##L*, ssize_t, T);                                      \
    ssize_t (*len)(struct _##L*);                                              \
    ssize_t (*cap)(struct _##L*);                                              \
    int (*append)(struct _##L*, T);                                            \
    int (*pop)(struct _##L*, ssize_t, T*);                                     \
    int (*insert)(struct _##L*, ssize_t, T);                                   \
    void (*sort)(struct _##L*, ssize_t, ssize_t, compare_func);                \
    int (*foreach)(struct _##L*, int (struct _##L*, ssize_t));                 \
    ssize_t (*find)(struct _##L*, T, compare_func);                            \
    int (*shuffle)(struct _##L*, ssize_t, ssize_t);                            \
    struct _##L* (*slice)(struct _##L*, ssize_t, ssize_t, ssize_t);            \
    void (*reverse)(struct _##L*);                                             \
    void (*clear)(struct _##L*);                                               \
    void (*destroy)(struct _##L*);                                             \
    int (*resize)(struct _##L*);                                               \
} _##L;                                                                        \
                                                                               \
typedef _##L *L;                                                               \
typedef int (*foreach_func_##L)(L, ssize_t);                                   \
                                                                               \
static int L##_resize(L list);                                                 \
static int L##_get(L list, ssize_t index, T* result);                          \
static int L##_set(L list, ssize_t index, T element);                          \
static ssize_t L##_size(L list);                                               \
static ssize_t L##_capacity(L list);                                           \
static int L##_append(L list, T element);                                      \
static int L##_pop(L list, ssize_t index, T *result);                          \
static int L##_insert(L list, ssize_t index, T element);                       \
static void L##_sort(L list, ssize_t left, ssize_t right, compare_func cmp);   \
static ssize_t L##_find(L list, T element, compare_func cmp);                  \
static int L##_foreach(L list, foreach_func_##L func);                         \
static L L##_slice(L list, ssize_t left, ssize_t right, ssize_t step);         \
static int L##_shuffle(L list, ssize_t left, ssize_t right);                   \
static void L##_reverse(L list);                                               \
static void L##_clear(L list);                                                 \
static void L##_destroy(L list);                                               \
                                                                               \
static ssize_t L##_size(L list)                                                \
{                                                                              \
    return list->size;                                                         \
}                                                                              \
                                                                               \
static ssize_t L##_capacity(L list)                                            \
{                                                                              \
    return list->capacity;                                                     \
}                                                                              \
                                                                               \
static L L##_new()                                                             \
{                                                                              \
    L result = (L)calloc(1, sizeof(_##L));                                     \
    if (result) {                                                              \
        result->data = NULL;                                                   \
        result->capacity = 0;                                                  \
        result->size = 0;                                                      \
                                                                               \
        /* Function APIs */                                                    \
        result->get = L##_get; result->set = L##_set;                          \
        result->len = L##_size; result->append = L##_append;                   \
        result->pop = L##_pop; result->insert = L##_insert;                    \
        result->clear = L##_clear; result->cap = L##_capacity;                 \
        result->destroy = L##_destroy; result->sort = L##_sort;                \
        result->foreach = L##_foreach; result->find = L##_find;                \
        result->shuffle = L##_shuffle; result->resize = L##_resize;            \
        result->reverse = L##_reverse; result->slice = L##_slice;              \
    }                                                                          \
    return result;                                                             \
}                                                                              \
                                                                               \
static int L##_get(L list, ssize_t index, T* result)                           \
{                                                                              \
    if (!list) return UNINITIALISED_ARRAY;                                     \
    if (list->len(list) <= index) return INDEX_OUT_OF_RANGE;                   \
    *result = list->data[index];                                               \
                                                                               \
    return 0;                                                                  \
}                                                                              \
                                                                               \
static int L##_set(L list, ssize_t index, T element)                           \
{                                                                              \
    if (!list) return UNINITIALISED_ARRAY;                                     \
    if (list->len(list) <= index) return INDEX_OUT_OF_RANGE;                   \
    list->data[index] = element;                                               \
    return 0;                                                                  \
}                                                                              \
                                                                               \
static int L##_append(L list, T element)                                       \
{                                                                              \
    int retCode;                                                               \
    if ((retCode = list->resize(list))) return retCode;                        \
    list->data[list->size++] = element;                                        \
    return retCode;                                                            \
}                                                                              \
                                                                               \
static int L##_pop(L list, ssize_t index, T *result)                           \
{                                                                              \
    if (index >= L##_size(list)) {                                             \
        return INDEX_OUT_OF_RANGE;                                             \
    }                                                                          \
    *result = list->data[index];                                               \
    memmove(&list->data[index], &list->data[index + 1],                        \
            (--(list->size) - index) * sizeof(T));                             \
    int retCode;                                                               \
    if ((retCode = list->resize(list))) return retCode;                        \
    return retCode;                                                            \
}                                                                              \
                                                                               \
static int L##_insert(L list, ssize_t index, T element)                        \
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
            (list->len(list) - index) * sizeof(T));                            \
    list->data[index] = element;                                               \
    return retCode;                                                            \
}                                                                              \
                                                                               \
static void L##_clear(L list)                                                  \
{                                                                              \
    if (!list->data) return;                                                   \
    free(list->data);                                                          \
    list->size = 0;                                                            \
    list->data = NULL;                                                         \
    list->capacity = 0;                                                        \
}                                                                              \
                                                                               \
static void L##_destroy(L list)                                                \
{                                                                              \
    if (!list) return;                                                         \
    list->clear(list);                                                         \
    free(list);                                                                \
}                                                                              \
                                                                               \
static void L##_sort(L list, ssize_t left, ssize_t right, compare_func cmp)    \
{                                                                              \
    if (left >= right) return;                                                 \
                                                                               \
    T pivot = list->data[left + (right - left) / 2];                           \
    ssize_t i = left;                                                          \
    ssize_t j = right;                                                         \
                                                                               \
    while (i <= j) {                                                           \
        while (cmp(&list->data[i], &pivot) < 0) i++;                           \
        while (cmp(&list->data[j], &pivot) > 0) j--;                           \
                                                                               \
        if (i <= j) {                                                          \
            T tmp = list->data[i];                                             \
            list->data[i] = list->data[j];                                     \
            list->data[j] = tmp;                                               \
            i++;                                                               \
            j--;                                                               \
        }                                                                      \
    }                                                                          \
    L##_sort(list, left, j, cmp);                                              \
    L##_sort(list, i, right, cmp);                                             \
}                                                                              \
                                                                               \
static int L##_foreach(L list, foreach_func_##L func)                          \
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
static ssize_t L##_find(L list, T element, compare_func cmp)                   \
{                                                                              \
    for (ssize_t i = 0; i < list->len(list); i++) {                            \
        if (cmp(&list->data[i], &element) == 0) return i;                      \
    }                                                                          \
    return -1;                                                                 \
}                                                                              \
                                                                               \
static int L##_shuffle(L list, ssize_t left, ssize_t right)                    \
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
        T tmp = list->data[i];                                                 \
        list->data[i] = list->data[j];                                         \
        list->data[j] = tmp;                                                   \
    }                                                                          \
    return 0;                                                                  \
}                                                                              \
                                                                               \
static void L##_reverse(L list)                                                \
{                                                                              \
    for (ssize_t i = 0; i < list->len(list) / 2; i++) {                        \
        T tmp = list->data[i];                                                 \
        list->data[i] = list->data[list->len(list) - i - 1];                   \
        list->data[list->len(list) - i - 1] = tmp;                             \
    }                                                                          \
}                                                                              \
                                                                               \
static L L##_slice(L list, ssize_t left, ssize_t right, ssize_t step)          \
{                                                                              \
    if (left < 0 || right >= list->len(list) || left > right)                  \
        return NULL;                                                           \
    if (step < 1) return NULL;                                                 \
    L new_list = L##_new();                                                    \
    if (step == 1) {                                                           \
        new_list->size = (right - left + 1) / step;                            \
        new_list->resize(new_list);                                            \
        memmove(new_list->data, &list->data[left],                             \
            (right - left + 1) * sizeof(T));                                   \
    } else {                                                                   \
        for (ssize_t i = left; i <= right; i += step)                          \
            if(new_list->append(new_list, list->data[i])) return NULL;         \
    }                                                                          \
    return new_list;                                                           \
}                                                                              \
                                                                               \
static int L##_resize(L list)                                                  \
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
    T *new_data = (T *) realloc(list->data, new_capacity * sizeof(T));         \
    if (new_data) {                                                            \
        list->data = new_data;                                                 \
        list->capacity = new_capacity;                                         \
        return 0;                                                              \
    }                                                                          \
    return MEMORY_ALLOCATION_ERR;                                              \
}                                                                              \
                                                                               \

#define LNEW(L) L##_new()

#endif //DATASTRUCTURES_LIST_H
