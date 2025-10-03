//
// Created by cheerfulmoss on 27/01/25.
//
// This implementation of a generic list was inspired by the work of
// TheBrokenPipe. Original source:
// https://github.com/TheBrokenPipe/CListTemplate/blob/main/list.h

#ifndef LIST_H
#define LIST_H

#include <pthread.h>
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
#define FAIL_EARLY() (abort())
#else
#define FAIL_EARLY() ((void)0)
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
            ERR_LOG(msg, ##__VA_ARGS__);                                       \
            FAIL_EARLY();                                                      \
        }                                                                      \
    } while (0)
#else
#define CRASH_IF(cond, msg, ...) ((void)0)
#endif

#ifndef LIST_NO_CONTRACTS
#define CRASH_IF_LOCKING(lst, cond, msg, ...)                                  \
    do {                                                                       \
        if (lst != NULL) {                                                     \
            pthread_rwlock_rdlock(&lst->rw_l);                                 \
        }                                                                      \
        if (cond) {                                                            \
            ERR_LOG(msg, ##__VA_ARGS__);                                       \
            if (lst != NULL) {                                                 \
                pthread_rwlock_unlock(&lst->rw_l);                             \
            }                                                                  \
            FAIL_EARLY();                                                      \
        }                                                                      \
        if (lst != NULL) {                                                     \
            pthread_rwlock_unlock(&lst->rw_l);                                 \
        }                                                                      \
    } while (0)
#else
#define CRASH_IF_LOCKING(cond, msg, ...) ((void)0)
#endif

#define CHECK_INDEX_OUT_OF_BOUNDS(lst, functionName, index, bound)             \
    CRASH_IF_LOCKING(lst, index >= bound,                                      \
                     functionName                                              \
                     " -> index (i=%zu) out of bounds (bound=%zu)",            \
                     index, bound)

#define CHECK_NOT_NULL(functionName, inputName, input)                         \
    CRASH_IF(input == NULL, functionName " -> " inputName " should not be "    \
                                         "null")

#define CHECK_NOT_ERR(functionName, calledName, input)                         \
    CRASH_IF(input != 0,                                                       \
             functionName " -> result of " calledName                          \
                          "() returned non 0 (%d)",                            \
             input)

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
    return ((rand() % 2) * 2) - 1;
}

#define NULL_SAFE(TYPE, VAR) TYPE(VAR)[static 1]
#define LIST_STRUCT(NAME) struct NAME##Struct
#define LIST_NULL_SAFE(NAME, VAR) NULL_SAFE(LIST_STRUCT(NAME), (VAR))

#define IMPORT_LIST_STRUCT(TYPE, NAME)                                         \
    typedef struct NAME##Struct {                                              \
        size_t capacity;                                                       \
        size_t length;                                                         \
        pthread_rwlock_t rw_l;                                                 \
        TYPE *data;                                                            \
    } NAME##Struct;                                                            \
    typedef LIST_STRUCT(NAME) * NAME;

#define IMPORT_LIST_CONSTRUCTOR(TYPE, NAME)                                    \
    static NAME NAME##_new() {                                                 \
        NAME lst = calloc(1, sizeof(NAME##Struct));                            \
        CHECK_NOT_NULL("create", "lst", lst);                                  \
        CHECK_NOT_ERR("create", "pthread_rwlock_init",                         \
                      pthread_rwlock_init(&lst->rw_l, NULL));                  \
        return lst;                                                            \
    }                                                                          \
    static void NAME##_destroy(NAME lst, void (*free_func)(TYPE)) {            \
        CHECK_NOT_NULL("destroy", "lst", lst);                                 \
        pthread_rwlock_wrlock(&lst->rw_l);                                     \
        for (size_t i = 0; i < lst->length; i++) {                             \
            if (free_func) {                                                   \
                free_func(lst->data[i]);                                       \
            }                                                                  \
        }                                                                      \
        free(lst->data);                                                       \
        pthread_rwlock_unlock(&lst->rw_l);                                     \
        pthread_rwlock_destroy(&lst->rw_l);                                    \
        free(lst);                                                             \
    }                                                                          \
    static void NAME##_clear(LIST_NULL_SAFE(NAME, lst)) {                      \
        CHECK_NOT_NULL("clear", "lst", lst);                                   \
        pthread_rwlock_wrlock(&lst->rw_l);                                     \
        lst->length = 0;                                                       \
        pthread_rwlock_unlock(&lst->rw_l);                                     \
    }                                                                          \
    static void NAME##_reserve(LIST_NULL_SAFE(NAME, lst), size_t cap) {        \
        CHECK_NOT_NULL("reserve", "lst", lst);                                 \
        pthread_rwlock_wrlock(&lst->rw_l);                                     \
        if (cap > lst->capacity) {                                             \
            lst->data =                                                        \
                _resize_array(lst->data, sizeof(TYPE), &lst->capacity, cap);   \
        }                                                                      \
        pthread_rwlock_unlock(&lst->rw_l);                                     \
    }

#define IMPORT_LIST_ACCESSORS(TYPE, NAME)                                      \
    static TYPE NAME##_get(LIST_NULL_SAFE(NAME, lst), size_t index) {          \
        CHECK_NOT_NULL("get", "lst", lst);                                     \
        pthread_rwlock_rdlock(&lst->rw_l);                                     \
        CHECK_INDEX_OUT_OF_BOUNDS(lst, "get", index, lst->length);             \
        const TYPE val = lst->data[index];                                     \
        pthread_rwlock_unlock(&lst->rw_l);                                     \
        return val;                                                            \
    }                                                                          \
    static void NAME##_set(LIST_NULL_SAFE(NAME, lst), size_t index,            \
                           TYPE value) {                                       \
        CHECK_NOT_NULL("set", "lst", lst);                                     \
        pthread_rwlock_wrlock(&lst->rw_l);                                     \
        CHECK_INDEX_OUT_OF_BOUNDS(lst, "set", index, lst->length);             \
        lst->data[index] = value;                                              \
        pthread_rwlock_unlock(&lst->rw_l);                                     \
    }                                                                          \
    static size_t NAME##_len(LIST_NULL_SAFE(NAME, lst)) {                      \
        CHECK_NOT_NULL("len", "lst", lst);                                     \
        pthread_rwlock_rdlock(&lst->rw_l);                                     \
        const size_t len = lst->length;                                        \
        pthread_rwlock_unlock(&lst->rw_l);                                     \
        return len;                                                            \
    }                                                                          \
    static size_t NAME##_capacity(LIST_NULL_SAFE(NAME, lst)) {                 \
        CHECK_NOT_NULL("capacity", "lst", lst);                                \
        pthread_rwlock_rdlock(&lst->rw_l);                                     \
        size_t cap = lst->capacity;                                            \
        pthread_rwlock_unlock(&lst->rw_l);                                     \
        return cap;                                                            \
    }

#define IMPORT_LIST_DYNAMIC_FUNCTIONS(TYPE, NAME)                              \
    static void NAME##__append_unlocked(NAME lst, TYPE value) {                \
        lst->data = _resize_array(lst->data, sizeof(TYPE), &lst->capacity,     \
                                  lst->length + 1);                            \
        lst->data[lst->length++] = value;                                      \
    }                                                                          \
    static void NAME##_append(LIST_NULL_SAFE(NAME, lst), TYPE value) {         \
        CHECK_NOT_NULL("append", "lst", lst);                                  \
        pthread_rwlock_wrlock(&lst->rw_l);                                     \
        NAME##__append_unlocked(lst, value);                                   \
        pthread_rwlock_unlock(&lst->rw_l);                                     \
    }                                                                          \
    static void NAME##_insert(LIST_NULL_SAFE(NAME, lst), size_t index,         \
                              TYPE value) {                                    \
        CHECK_NOT_NULL("insert", "lst", lst);                                  \
        pthread_rwlock_wrlock(&lst->rw_l);                                     \
        if (index == lst->length) {                                            \
            NAME##__append_unlocked(lst, value);                               \
            pthread_rwlock_unlock(&lst->rw_l);                                 \
            return;                                                            \
        }                                                                      \
        CHECK_INDEX_OUT_OF_BOUNDS(lst, "insert", index, lst->length);          \
        lst->data = _resize_array(lst->data, sizeof(TYPE), &lst->capacity,     \
                                  lst->length + 1);                            \
        memmove(&(lst->data[index + 1]), &(lst->data[index]),                  \
                (lst->length - index) * sizeof(TYPE));                         \
        lst->length++;                                                         \
        lst->data[index] = value;                                              \
        pthread_rwlock_unlock(&lst->rw_l);                                     \
    }                                                                          \
    static TYPE NAME##_pop(LIST_NULL_SAFE(NAME, lst), size_t index) {          \
        CHECK_NOT_NULL("pop", "lst", lst);                                     \
        CHECK_INDEX_OUT_OF_BOUNDS(lst, "pop", index, lst->length);             \
        pthread_rwlock_wrlock(&lst->rw_l);                                     \
        TYPE ret = lst->data[index];                                           \
        lst->length--;                                                         \
        if (index < lst->length) {                                             \
            memmove(&(lst->data[index]), &(lst->data[index + 1]),              \
                    (lst->length - index) * sizeof(TYPE));                     \
        }                                                                      \
        pthread_rwlock_unlock(&lst->rw_l);                                     \
        return ret;                                                            \
    }                                                                          \
    static void NAME##_shrink_to_fit(LIST_NULL_SAFE(NAME, lst)) {              \
        CHECK_NOT_NULL("shrink_to_fit", "lst", lst);                           \
        pthread_rwlock_wrlock(&lst->rw_l);                                     \
        TYPE *tmp = _resize_array(lst->data, sizeof(TYPE), &lst->capacity,     \
                                  lst->length);                                \
        CHECK_NOT_NULL("shrink_to_fit", "tmp", tmp);                           \
        lst->data = tmp;                                                       \
        lst->capacity = lst->length;                                           \
        pthread_rwlock_unlock(&lst->rw_l);                                     \
    }

#define IMPORT_LIST_MAPS(TYPE, NAME)                                           \
    typedef TYPE (*NAME##_map_func)(TYPE);                                     \
    typedef TYPE (*NAME##_enum_map_func)(size_t, TYPE);                        \
    static void NAME##_map(LIST_NULL_SAFE(NAME, lst), NAME##_map_func f) {     \
        CHECK_NOT_NULL("map", "lst", lst);                                     \
        CHECK_NOT_NULL("map", "f", f);                                         \
        pthread_rwlock_wrlock(&lst->rw_l);                                     \
        for (size_t i = 0; i < lst->length; i++) {                             \
            lst->data[i] = f(lst->data[i]);                                    \
        }                                                                      \
        pthread_rwlock_unlock(&lst->rw_l);                                     \
    }                                                                          \
    static void NAME##_enum_map(LIST_NULL_SAFE(NAME, lst),                     \
                                NAME##_enum_map_func f) {                      \
        CHECK_NOT_NULL("map", "lst", lst);                                     \
        CHECK_NOT_NULL("map", "f", f);                                         \
        pthread_rwlock_wrlock(&lst->rw_l);                                     \
        for (size_t i = 0; i < lst->length; i++) {                             \
            lst->data[i] = f(i, lst->data[i]);                                 \
        }                                                                      \
        pthread_rwlock_unlock(&lst->rw_l);                                     \
    }

/* The two functions here may look the same but there is an important
 * distinction. If you want to be able to do list_extend_list(l1, l1), you
 * cannot pass down to the array extend as it will lose the pointer if a resize
 * is required.
 */
#define IMPORT_LIST_EXTENSORS(TYPE, NAME)                                      \
    static void NAME##_extend_array(LIST_NULL_SAFE(NAME, dst),                 \
                                    NULL_SAFE(TYPE, src), size_t length) {     \
        CHECK_NOT_NULL("extend array", "dst", dst);                            \
        CHECK_NOT_NULL("extend array", "src", src);                            \
        pthread_rwlock_wrlock(&dst->rw_l);                                     \
        if (length == 0) {                                                     \
            return;                                                            \
        }                                                                      \
        dst->data = _resize_array(dst->data, sizeof(TYPE), &dst->capacity,     \
                                  dst->length + length);                       \
        memmove(&(dst->data[dst->length]), src, length * sizeof(TYPE));        \
        dst->length += length;                                                 \
        pthread_rwlock_unlock(&dst->rw_l);                                     \
    }                                                                          \
    static void NAME##_extend_##NAME(LIST_NULL_SAFE(NAME, dst),                \
                                     LIST_NULL_SAFE(NAME, src)) {              \
        CHECK_NOT_NULL("extend list", "dst", dst);                             \
        CHECK_NOT_NULL("extend list", "src", src);                             \
        pthread_rwlock_wrlock(&dst->rw_l);                                     \
        pthread_rwlock_rdlock(&src->rw_l);                                     \
        if (src->length == 0) {                                                \
            return;                                                            \
        }                                                                      \
        dst->data = _resize_array(dst->data, sizeof(TYPE), &dst->capacity,     \
                                  dst->length + src->length);                  \
        memmove(&(dst->data[dst->length]), &(src->data[0]),                    \
                src->length * sizeof(TYPE));                                   \
        dst->length += src->length;                                            \
        pthread_rwlock_unlock(&src->rw_l);                                     \
        pthread_rwlock_unlock(&dst->rw_l);                                     \
    }

#define IMPORT_LIST_COMPARATORS(TYPE, NAME)                                    \
    typedef int (*NAME##_comp_func)(const void *, const void *);               \
    static void NAME##_qsort(LIST_NULL_SAFE(NAME, lst), size_t start,          \
                             size_t end, NAME##_comp_func f) {                 \
        CHECK_NOT_NULL("qsort", "lst", lst);                                   \
        CHECK_NOT_NULL("qsort", "f", f);                                       \
        pthread_rwlock_wrlock(&lst->rw_l);                                     \
        qsort(&(lst->data[start]), end - start, sizeof(TYPE), f);              \
        pthread_rwlock_unlock(&lst->rw_l);                                     \
    }                                                                          \
    static ssize_t NAME##_find(LIST_NULL_SAFE(NAME, lst), TYPE value,          \
                               NAME##_comp_func f) {                           \
        CHECK_NOT_NULL("find", "lst", lst);                                    \
        CHECK_NOT_NULL("find", "f", f);                                        \
        pthread_rwlock_rdlock(&lst->rw_l);                                     \
        for (size_t i = 0; i < lst->length; i++) {                             \
            if (f((void *)(&lst->data[i]), (void *)(&value)) == 0) {           \
                pthread_rwlock_unlock(&lst->rw_l);                             \
                return i;                                                      \
            }                                                                  \
        }                                                                      \
        pthread_rwlock_unlock(&lst->rw_l);                                     \
        return -1;                                                             \
    }                                                                          \
    static size_t NAME##_count(LIST_NULL_SAFE(NAME, lst), TYPE value,          \
                               NAME##_comp_func f) {                           \
        CHECK_NOT_NULL("count", "lst", lst);                                   \
        CHECK_NOT_NULL("count", "f", f);                                       \
        pthread_rwlock_rdlock(&lst->rw_l);                                     \
        size_t count = 0;                                                      \
        for (size_t i = 0; i < lst->length; i++) {                             \
            if (f((void *)(&lst->data[i]), (void *)(&value)) == 0) {           \
                count++;                                                       \
            }                                                                  \
        }                                                                      \
        pthread_rwlock_unlock(&lst->rw_l);                                     \
        return count;                                                          \
    }                                                                          \
    static void NAME##_dumb_shuffle(LIST_NULL_SAFE(NAME, lst), size_t start,   \
                                    size_t end) {                              \
        CHECK_NOT_NULL("dumb_shuffle", "lst", lst);                            \
        pthread_rwlock_wrlock(&lst->rw_l);                                     \
        qsort(&(lst->data[start]), end - start, sizeof(TYPE), _shuffle_func);  \
        pthread_rwlock_unlock(&lst->rw_l);                                     \
    }

#define IMPORT_LIST_DEBUG_PRINT(TYPE, NAME, FORMAT)                            \
    static void NAME##_print(LIST_NULL_SAFE(NAME, lst)) {                      \
        CHECK_NOT_NULL("print", "lst", lst);                                   \
        pthread_rwlock_rdlock(&lst->rw_l);                                     \
        printf("[");                                                           \
        for (size_t i = 0; i < lst->length; i++) {                             \
            printf(FORMAT, lst->data[i]);                                      \
            if (i < lst->length - 1)                                           \
                printf(", ");                                                  \
        }                                                                      \
        printf("]\n");                                                         \
        pthread_rwlock_unlock(&lst->rw_l);                                     \
    }

#define IMPORT_LIST_DEBUG_DUMP(TYPE, NAME, FORMAT)                             \
    static void NAME##_dump(LIST_NULL_SAFE(NAME, lst)) {                       \
        CHECK_NOT_NULL("dump", "lst", lst);                                    \
        pthread_rwlock_rdlock(&lst->rw_l);                                     \
        printf("[DEBUG] Dumping " #NAME " at %p:\n", (void *)lst);             \
        printf("  - Length:       %zu\n", lst->length);                        \
        printf("  - Capacity:     %zu\n", lst->capacity);                      \
        printf("  - Data pointer: %p\n", (void *)lst->data);                   \
        printf("  - Elements:     [");                                         \
        for (size_t i = 0; i < lst->length; ++i) {                             \
            printf(FORMAT, lst->data[i]);                                      \
            if (i < lst->length - 1)                                           \
                printf(", ");                                                  \
        }                                                                      \
        printf("]\n");                                                         \
        pthread_rwlock_unlock(&lst->rw_l);                                     \
    }

#define IMPORT_LIST_DEBUG_TOOLS(TYPE, NAME, FORMAT)                            \
    IMPORT_LIST_DEBUG_PRINT(TYPE, NAME, FORMAT)                                \
    IMPORT_LIST_DEBUG_DUMP(TYPE, NAME, FORMAT)

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
