//
// Created by cheerfulmoss on 27/01/25.
//
// This implementation of a generic list was inspired by the work of TheBrokenPipe.
// Original source: https://github.com/TheBrokenPipe/CListTemplate/blob/main/list.h

#ifndef LIST_H
#define LIST_H

#include <assert.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

size_t next_power_of_two(size_t n) {
    if (n == 0) return 1;
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
#if SIZE_MAX > 0xFFFFFFFF
    n |= n >> 32;
#endif
    return n + 1;
}

#define IMPORT_LIST(TYPE, LABEL, FORMAT_SPECIFIER)\
\
typedef struct _##LABEL *LABEL;\
typedef void (*LABEL##_enumerate_for_each_func)(size_t, TYPE);\
typedef TYPE (*LABEL##_enumerate_map_func)(LABEL, size_t, TYPE);\
/* TYPE a < TYPE b = -1, TYPE b < TYPE a = 1, TYPE a == TYPE b = 0 */\
typedef int (*LABEL##_compare_func)(TYPE, TYPE);\
\
typedef struct _##LABEL {\
    size_t capacity;\
    size_t length;\
    TYPE *data;\
    \
    size_t (*len)(LABEL);\
    TYPE (*get)(LABEL, size_t);\
    LABEL (*set)(LABEL, size_t, TYPE);\
    LABEL (*resize)(LABEL);\
    LABEL (*push)(LABEL, TYPE);\
    void (*destroy)(LABEL);\
    LABEL (*insert)(LABEL, size_t, TYPE);\
    LABEL (*enumerate_for_each)(LABEL, LABEL##_enumerate_for_each_func);\
    LABEL (*enumerate_map)(LABEL, LABEL##_enumerate_map_func);\
    LABEL (*extend_list)(LABEL, LABEL);\
    LABEL (*extend_array)(LABEL, TYPE *, size_t);\
    LABEL (*__allocate__)(LABEL, size_t);\
    int (*__dumb_random__)(TYPE, TYPE);\
    TYPE (*pop)(LABEL, size_t);\
    LABEL (*display)(LABEL);\
    LABEL (*slice)(LABEL, size_t, size_t, size_t);\
    LABEL (*copy)(LABEL);\
    LABEL (*sort)(LABEL, size_t, size_t, LABEL##_compare_func);\
    LABEL (*shuffle)(LABEL);\
    size_t (*find)(LABEL, TYPE, LABEL##_compare_func);\
    LABEL (*clear)(LABEL);\
    size_t (*count)(LABEL, TYPE, LABEL##_compare_func);\
} _##LABEL;\
\
static size_t LABEL##_len(LABEL);\
static TYPE LABEL##_get(LABEL, size_t);\
static LABEL LABEL##_set(LABEL, size_t, TYPE);\
static LABEL LABEL##_resize(LABEL);\
static LABEL LABEL##_push(LABEL, TYPE);\
static void LABEL##_destroy(LABEL);\
static LABEL LABEL##_insert(LABEL, size_t, TYPE);\
static LABEL LABEL##_enumerate_for_each(LABEL, LABEL##_enumerate_for_each_func);\
static LABEL LABEL##_enumerate_map(LABEL, LABEL##_enumerate_map_func);\
static LABEL LABEL##_extend_list(LABEL, LABEL);\
static LABEL LABEL##_extend_array(LABEL, TYPE *, size_t);\
static LABEL LABEL##__allocate__(LABEL, size_t);\
static int LABEL##__dumb_random__(TYPE, TYPE);\
static TYPE LABEL##_pop(LABEL, size_t);\
static LABEL LABEL##_display(LABEL);\
static LABEL LABEL##_slice(LABEL, size_t, size_t, size_t);\
static LABEL LABEL##_copy(LABEL);\
static LABEL LABEL##_sort(LABEL, size_t, size_t, LABEL##_compare_func);\
static LABEL LABEL##_shuffle(LABEL);\
static size_t LABEL##_find(LABEL, TYPE, LABEL##_compare_func);\
static LABEL LABEL##_clear(LABEL);\
static size_t LABEL##_count(LABEL, TYPE, LABEL##_compare_func);\
\
static LABEL LABEL##_new() {\
    LABEL self = (LABEL)calloc(1, sizeof(_##LABEL));\
    assert(self != NULL);\
    self->data = NULL;\
    self->capacity = 0;\
    self->length = 0;\
    self->len = LABEL##_len;\
    self->get = LABEL##_get;\
    self->set = LABEL##_set;\
    self->resize = LABEL##_resize;\
    self->push = LABEL##_push;\
    self->destroy = LABEL##_destroy;\
    self->insert = LABEL##_insert;\
    self->enumerate_for_each = LABEL##_enumerate_for_each;\
    self->enumerate_map = LABEL##_enumerate_map;\
    self->__allocate__ = LABEL##__allocate__;\
    self->extend_list = LABEL##_extend_list;\
    self->extend_array = LABEL##_extend_array;\
    self->pop = LABEL##_pop;\
    self->display = LABEL##_display;\
    self->slice = LABEL##_slice;\
    self->copy = LABEL##_copy;\
    self->sort = LABEL##_sort;\
    self->shuffle = LABEL##_shuffle;\
    self->__dumb_random__ = LABEL##__dumb_random__;\
    self->find = LABEL##_find;\
    self->clear = LABEL##_clear;\
    self->count = LABEL##_count;\
    \
    struct timespec ts;\
    clock_gettime(CLOCK_REALTIME, &ts);\
    srand(ts.tv_nsec ^ ts.tv_sec);\
    return self;\
}\
\
static size_t LABEL##_len(LABEL self) { \
    assert(self != NULL);\
    return self->length; \
} \
static TYPE LABEL##_get(LABEL self, size_t index) {\
    assert(self != NULL);\
    assert(index < self->len(self));\
    return self->data[index];\
}\
static LABEL LABEL##_set(LABEL self, size_t index, TYPE value) { \
    assert(self != NULL);\
    if (index < 0) {\
    index = self->len(self) - index;\
    }\
    assert(index < self->len(self) && index >= 0);\
    self->data[index] = value;\
    return self;\
}\
static LABEL LABEL##_resize(LABEL self) {\
    assert(self != NULL);\
    size_t new_cap = self->capacity;\
    size_t lower_threshold = new_cap / 4;\
    if (self->len(self) <= lower_threshold) {\
        new_cap /= 2;\
    } else if (self->len(self) >= self->capacity) {\
        new_cap *= 2;\
    }\
    new_cap = new_cap == 0 ? 1 : new_cap;\
    TYPE *new_data = (TYPE *)realloc(self->data, new_cap * sizeof(TYPE));\
    assert(new_data != NULL);\
    self->data = new_data;\
    self->capacity = new_cap;\
    return self;\
}\
static LABEL LABEL##__allocate__(LABEL self, size_t new_capacity) {\
    assert(self != NULL);\
    assert(new_capacity != 0);\
    if (new_capacity < self->len(self)) {\
        return self;\
    }\
    TYPE *new_data = (TYPE *)realloc(self->data, new_capacity * sizeof(TYPE));\
    assert(new_data != NULL);\
    self->data = new_data;\
    self->capacity = new_capacity;\
    return self;\
}\
static LABEL LABEL##_push(LABEL self, TYPE value) {\
    assert(self != NULL);\
    self->resize(self);\
    self->data[self->len(self)] = value;\
    self->length++;\
    return self;\
}\
static LABEL LABEL##_insert(LABEL self, size_t index, TYPE value) {\
    assert(self != NULL);\
    self->resize(self);\
    memmove(&self->data[index + 1], &self->data[index], sizeof(TYPE) * (self->len(self) - index));\
    self->data[index] = value;\
    self->length++;\
    return self;\
}\
static LABEL LABEL##_enumerate_for_each(LABEL self, LABEL##_enumerate_for_each_func func) {\
    assert(self != NULL);\
    for (size_t i = 0; i < self->len(self); i++) {\
        func(i, self->get(self, i));\
    }\
    return self;\
}\
static LABEL LABEL##_enumerate_map(LABEL self, LABEL##_enumerate_map_func func) {\
    assert(self != NULL);\
    for (size_t i = 0; i < self->len(self); i++) {\
        TYPE new_val = func(self, i, self->get(self, i));\
        self->set(self, i, new_val);\
    }\
    return self;\
}\
static LABEL LABEL##_extend_array(LABEL self, TYPE *other, size_t length) {\
    assert(self != NULL);\
    if (self->len(self) + length > self->capacity) {\
        self->__allocate__(self, next_power_of_two(self->len(self) + length));\
    }\
    memmove(&self->data[self->len(self)], other, length * sizeof(TYPE));\
    self->length += length;\
    return self;\
}\
static LABEL LABEL##_extend_list(LABEL self, LABEL other) {\
    assert(self != NULL);\
    return self->extend_array(self, other->data, other->len(other));\
}\
static TYPE LABEL##_pop(LABEL self, size_t index) {\
    assert(self != NULL);\
    TYPE value = self->get(self, index);\
    self->length--;\
    if (index == self->len(self)) {\
        return value;\
    }\
    memmove(&self->data[index], &self->data[index + 1], (self->len(self) - index) * sizeof(TYPE));\
    return value;\
}\
static LABEL LABEL##_display(LABEL self) {\
    assert(self != NULL);\
    char test[] = FORMAT_SPECIFIER;\
    assert(strlen(test) != 0);\
    for (size_t i = 0; i < self->len(self); i++) {\
        if (i == 0) {\
            printf("{");\
        }\
        TYPE element = self->get(self, i);\
        printf(FORMAT_SPECIFIER, element);\
        if (i == self->len(self) - 1) {\
            printf("}");\
        } else {\
            printf(", ");\
        }\
    }\
    return self;\
}\
static LABEL LABEL##_slice(LABEL self, size_t start, size_t stop, size_t step) {\
    assert(self != NULL);\
    assert(self->len(self) >= stop && start <= stop);\
    LABEL sliced = LABEL##_new();\
    for (size_t i = start; i < stop; i += step) {\
        sliced->push(sliced, self->get(self, i));\
    }\
    return sliced;\
}\
static LABEL LABEL##_copy(LABEL self) {\
    assert(self != NULL);\
    LABEL new_list = LABEL##_new();\
    new_list->length = self->length;\
    new_list->__allocate__(new_list, self->capacity);\
    memcpy(new_list->data, self->data, self->len(self) * sizeof(TYPE));\
    return new_list;\
}\
static LABEL LABEL##_sort(LABEL self, size_t left, size_t right, LABEL##_compare_func comp) {\
    assert(self != NULL);\
    assert(self->data != NULL);\
    assert(left < self->len(self));\
    assert(right < self->len(self));\
    if (left >= right || right >= self->len(self)) return self;\
    \
    size_t mid = left + (right - left) / 2;\
    assert(mid < self->len(self));\
    int pivot = self->data[mid];\
    \
    size_t i = left;\
    size_t j = right;\
    while (i <= j) {\
        while (i < self->len(self) && comp(self->get(self, i), pivot) < 0) i++;\
        while (j < self->len(self) && comp(self->get(self, j), pivot) > 0) {\
            if (j == 0) break;\
            j--;\
        }\
        if (i <= j) {\
            int tmp = self->get(self, i);\
            self->set(self, i, self->get(self, j));\
            self->set(self, j, tmp);\
            i++;\
            if (j > 0) j--;\
        }\
    }\
    \
    if (left < j) self->sort(self, left, j, comp);\
    if (i < right) self->sort(self, i, right, comp);\
    return self;\
}\
static int LABEL##__dumb_random__(TYPE a, TYPE b) {\
    (void)a; (void)b;\
    return (rand() % 3) - 1;\
}\
static LABEL LABEL##_shuffle(LABEL self) {\
    assert(self != NULL);\
    assert(self->data != NULL);\
    return self->sort(self, 0, self->len(self) - 1, self->__dumb_random__);\
}\
static size_t LABEL##_find(LABEL self, TYPE value, LABEL##_compare_func comp) {\
    assert(self != NULL);\
    assert(self->data != NULL);\
    for (size_t index = 0; index < self->len(self); index++) {\
        if (comp(self->get(self, index), value) == 0) {\
            return index;\
        }\
    }\
    return self->len(self);\
}\
static LABEL LABEL##_clear(LABEL self) {\
    assert(self != NULL);\
    assert(self->data != NULL);\
    free(self->data);\
    self->data = NULL;\
    self->length = 0;\
    self->capacity = 0;\
    return self;\
}\
static void LABEL##_destroy(LABEL self) {\
    assert(self != NULL);\
    if (self->data != NULL) free(self->data);\
    free(self);\
}\
static size_t LABEL##_count(LABEL self, TYPE value, LABEL##_compare_func comp) {\
    assert(self != NULL);\
    assert(self->data != NULL);\
    size_t count = 0;\
    for (size_t index = 0; index < self->len(self); index++) {\
        if (comp(self->get(self, index), value) == 0) {\
            count++;\
        }\
    }\
    return count;\
}\

#endif //LIST_H
