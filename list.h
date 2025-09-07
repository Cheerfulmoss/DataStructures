//
// Created by cheerfulmoss on 27/01/25.
//
// This implementation of a generic list was inspired by the work of
// TheBrokenPipe. Original source:
// https://github.com/TheBrokenPipe/CListTemplate/blob/main/list.h

#ifndef LIST_H
#define LIST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef LIST_NO_ERR_LOG
#define ERR_LOG(msg, ...)                                                      \
    fprintf(stderr, "[%s:%d: (bottom) %s] " msg "\n", __FILE__, __LINE__,      \
            __func__, ##__VA_ARGS__)
#else
#define ERR_LOG(msg, ...) ((void)0)
#endif

#ifndef LIST_LOG_ONLY
#define EXIT_FAILURE() (abort())
#else
#define EXIT_FAILURE() ((void)0)
#endif

/* There is no error propagation if NO_CONTRACTS is defined, if an error occurs
 * (ie something is null when it shouldn't be, something that is assumed never
 * to fail fails) it may or may not fail in a predictable way. It could still
 * segfault or it may act weird.
 */
#ifndef LIST_NO_CONTRACTS
#define CRASH_IF(cond, msg, ...)                                               \
    do {                                                                       \
        if (cond) {                                                            \
                                                                               \
            ERR_LOG(msg, ##__VA_ARGS__);                                       \
            EXIT_FAILURE();                                                    \
        }                                                                      \
    } while (0)
#else
#define CRASH_IF(cond, msg, ...) ((void)0)
#endif

#define CHECK_INDEX_OUT_OF_BOUNDS(functionName, index, bound)                  \
    CRASH_IF(index >= bound,                                                   \
             functionName " -> index (i=%zu) out of bounds (bound=%zu)",       \
             index, bound)

#define CHECK_NOT_NULL(functionName, inputName, input)                         \
    CRASH_IF(input == NULL, functionName " -> " inputName " should not be "    \
                                         "null")

static inline size_t _fit_cap_to_size(const size_t cap, const size_t size) {
    if (cap == 0)
        return size;
    return ((size + cap - 1) / cap) * cap;
}

static inline void *_resize_array(void *arr, const size_t elemSize, size_t *cap,
                                  const size_t size) {
    const size_t newCap = _fit_cap_to_size(*cap, size);
    void *newDat = arr;
    if (newCap != *cap) {
        newDat = reallocarray(arr, newCap, elemSize);
        CHECK_NOT_NULL("_resize_array", "newDat", newDat);
        *cap = newCap;
    }
    return newDat;
}

static inline int _shuffle_func(const void *a, const void *b) {
    (void)a;
    (void)b;
    return (rand() % 3) - 1;
}

#define IMPORT_LIST_STRUCT(TYPE, NAME)                                         \
    typedef struct NAME##Struct {                                              \
        size_t capacity;                                                       \
        size_t length;                                                         \
        TYPE *data;                                                            \
    } NAME##Struct;                                                            \
    typedef struct NAME##Struct *NAME;

#define IMPORT_LIST_CONSTRUCTOR(TYPE, NAME)                                    \
    static NAME NAME##_new() {                                                 \
        NAME lst = calloc(1, sizeof(NAME##Struct));                            \
        CHECK_NOT_NULL("create", "lst", lst);                                  \
        lst->data = NULL;                                                      \
        lst->capacity = 0;                                                     \
        lst->length = 0;                                                       \
        return lst;                                                            \
    }                                                                          \
    static void NAME##_destroy(NAME lst) {                                     \
        CHECK_NOT_NULL("destroy", "lst", lst);                                 \
        if (lst->data != NULL) {                                               \
            free(lst->data);                                                   \
        }                                                                      \
        free(lst);                                                             \
    }                                                                          \
    static void NAME##_clear(NAME lst) { lst->length = 0; }

#define IMPORT_LIST_ACCESSORS(TYPE, NAME)                                      \
    static TYPE NAME##_get(NAME lst, size_t index) {                           \
        CHECK_NOT_NULL("get", "lst", lst);                                     \
        CHECK_INDEX_OUT_OF_BOUNDS("get", index, lst->length);                  \
        return lst->data[index];                                               \
    }                                                                          \
    static void NAME##_set(NAME lst, size_t index, TYPE value) {               \
        CHECK_NOT_NULL("set", "lst", lst);                                     \
        CHECK_INDEX_OUT_OF_BOUNDS("set", index, lst->length);                  \
        lst->data[index] = value;                                              \
    }                                                                          \
    static size_t NAME##_len(NAME lst) {                                       \
        CHECK_NOT_NULL("len", "lst", lst);                                     \
        return lst->length;                                                    \
    }

#define IMPORT_LIST_DYNAMIC_FUNCTIONS(TYPE, NAME)                              \
    static void NAME##_append(NAME lst, TYPE value) {                          \
        CHECK_NOT_NULL("append", "lst", lst);                                  \
        lst->data = _resize_array(lst->data, sizeof(TYPE), &lst->capacity,     \
                                  lst->length + 1);                            \
        lst->data[lst->length++] = value;                                      \
    }                                                                          \
                                                                               \
    static void NAME##_insert(NAME lst, size_t index, TYPE value) {            \
        CHECK_NOT_NULL("insert", "lst", lst);                                  \
        if (index == lst->length) {                                            \
            NAME##_append(lst, value);                                         \
            return;                                                            \
        }                                                                      \
        CHECK_INDEX_OUT_OF_BOUNDS("insert", index, lst->length);               \
        lst->data = _resize_array(lst->data, sizeof(TYPE), &lst->capacity,     \
                                  lst->length + 1);                            \
        memmove(&(lst->data[index + 1]), &(lst->data[index]),                  \
                (lst->length - index) * sizeof(TYPE));                         \
        lst->length++;                                                         \
        lst->data[index] = value;                                              \
    }                                                                          \
    static TYPE NAME##_pop(NAME lst, size_t index) {                           \
        CHECK_NOT_NULL("pop", "lst", lst);                                     \
        CHECK_INDEX_OUT_OF_BOUNDS("pop", index, lst->length);                  \
        TYPE ret = lst->data[index];                                           \
        lst->length--;                                                         \
        if (index < lst->length) {                                             \
            memmove(&(lst->data[index]), &(lst->data[index + 1]),              \
                    (lst->length - index) * sizeof(TYPE));                     \
        }                                                                      \
        return ret;                                                            \
    }

#define IMPORT_LIST_MAPS(TYPE, NAME)                                           \
    typedef TYPE (*NAME##_map_func)(TYPE);                                     \
    typedef TYPE (*NAME##_enum_map_func)(size_t, TYPE);                        \
    static void NAME##_map(NAME lst, NAME##_map_func f) {                      \
        CHECK_NOT_NULL("map", "lst", lst);                                     \
        CHECK_NOT_NULL("map", "f", f);                                         \
        for (size_t i = 0; i < lst->length; i++) {                             \
            lst->data[i] = f(lst->data[i]);                                    \
        }                                                                      \
    }                                                                          \
    static void NAME##_enum_map(NAME lst, NAME##_enum_map_func f) {            \
        CHECK_NOT_NULL("map", "lst", lst);                                     \
        CHECK_NOT_NULL("map", "f", f);                                         \
        for (size_t i = 0; i < lst->length; i++) {                             \
            lst->data[i] = f(i, lst->data[i]);                                 \
        }                                                                      \
    }

/* The two functions here may look the same but there is an important
 * distinction If you want to be able to do list_extend_list(l1, l1), you cannot
 * pass down to the array extend as it will lose the pointer if a resize is
 * required.
 */
#define IMPORT_LIST_EXTENSORS(TYPE, NAME)                                      \
    static void NAME##_extend_array(NAME dst, TYPE *src, size_t length) {      \
        CHECK_NOT_NULL("extend array", "dst", dst);                            \
        CHECK_NOT_NULL("extend array", "src", src);                            \
        if (length == 0) {                                                     \
            return;                                                            \
        }                                                                      \
        dst->data = _resize_array(dst->data, sizeof(TYPE), &dst->capacity,     \
                                  dst->length + length);                       \
        memmove(&(dst->data[dst->length]), src, length * sizeof(TYPE));        \
        dst->length += length;                                                 \
    }                                                                          \
    static void NAME##_extend_##NAME(NAME dst, NAME src) {                     \
        CHECK_NOT_NULL("extend list", "dst", dst);                             \
        CHECK_NOT_NULL("extend list", "src", src);                             \
        if (src->length == 0) {                                                \
            return;                                                            \
        }                                                                      \
        dst->data = _resize_array(dst->data, sizeof(TYPE), &dst->capacity,     \
                                  dst->length + src->length);                  \
        memmove(&(dst->data[dst->length]), &(src->data[0]),                    \
                src->length * sizeof(TYPE));                                   \
        dst->length += src->length;                                            \
    }

#define IMPORT_LIST_COMPARATORS(TYPE, NAME)                                    \
    typedef int (*NAME##_comp_func)(const void *, const void *);               \
    static void NAME##_qsort(NAME lst, size_t start, size_t end,               \
                             NAME##_comp_func f) {                             \
        qsort(&(lst->data[start]), end, sizeof(TYPE), f);                      \
    }                                                                          \
    static ssize_t NAME##_find(NAME lst, TYPE value, NAME##_comp_func f) {     \
        for (size_t i = 0; i < lst->length; i++) {                             \
            if (f((void *)(&lst->data[i]), (void *)(&value)) == 0) {           \
                return i;                                                      \
            }                                                                  \
        }                                                                      \
        return -1;                                                             \
    }                                                                          \
    static size_t NAME##_count(NAME lst, TYPE value, NAME##_comp_func f) {     \
        size_t count = 0;                                                      \
        for (size_t i = 0; i < lst->length; i++) {                             \
            if (f((void *)(&lst->data[i]), (void *)(&value)) == 0) {           \
                count++;                                                       \
            }                                                                  \
        }                                                                      \
        return count;                                                          \
    }                                                                          \
    static void NAME##_dumb_shuffle(NAME lst, size_t start, size_t end) {      \
        qsort(&(lst->data[start]), end, sizeof(TYPE), _shuffle_func);          \
    }

#define IMPORT_LIST_BASIC(TYPE, NAME)                                          \
    IMPORT_LIST_STRUCT(TYPE, NAME);                                            \
    IMPORT_LIST_CONSTRUCTOR(TYPE, NAME);                                       \
    IMPORT_LIST_ACCESSORS(TYPE, NAME);                                         \
    IMPORT_LIST_DYNAMIC_FUNCTIONS(TYPE, NAME);

#define IMPORT_LIST_DYNAMIC_ARRAY(TYPE, NAME)                                  \
    IMPORT_LIST_STRUCT(TYPE, NAME);                                            \
    IMPORT_LIST_CONSTRUCTOR(TYPE, NAME);                                       \
    IMPORT_LIST_ACCESSORS(TYPE, NAME);                                         \
    IMPORT_LIST_DYNAMIC_FUNCTIONS(TYPE, NAME);                                 \
    IMPORT_LIST_EXTENSORS(TYPE, NAME);

#define IMPORT_LIST_ALL(TYPE, NAME)                                            \
    IMPORT_LIST_STRUCT(TYPE, NAME);                                            \
    IMPORT_LIST_CONSTRUCTOR(TYPE, NAME);                                       \
    IMPORT_LIST_ACCESSORS(TYPE, NAME);                                         \
    IMPORT_LIST_DYNAMIC_FUNCTIONS(TYPE, NAME);                                 \
    IMPORT_LIST_EXTENSORS(TYPE, NAME);                                         \
    IMPORT_LIST_MAPS(TYPE, NAME);                                              \
    IMPORT_LIST_COMPARATORS(TYPE, NAME);

#endif // LIST_H
