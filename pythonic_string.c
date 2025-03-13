//
// Created by cheerfulmoss on 02/03/25.
//

#include "pythonic_string.h"

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>

static void crash_if_impl(const bool condition, const char *message, const char *file, int line) {
    if (!condition) {
        fprintf(stderr, "Fatal error: %s\nFile: %s, Line: %d\n", message, file, line);
        abort();
    }
}

#define ASSERT_CRASH(cond, msg) crash_if_impl((cond), (msg), (__FILE__), (__LINE__))
#define MIN(a, b) do {((a) < (b) ? (a) : (b))} while (0)
#define MAX(a, b) do {((a) > (b) ? (a) : (b))} while (0)

#define STANDARD_ASSERTS(str_identifier) \
    do { \
        ASSERT_CRASH((str_identifier) != NULL, "String may be uninitialised or previously freed.");\
        ASSERT_CRASH((str_identifier)->rwlock != NULL, "String mutex is missing."); \
        ASSERT_CRASH((str_identifier)->cookie == COOKIE, "String cookie not present. String may have been freed.");\
    } while (0) \

const uint16_t COOKIE = 0xD21B;

static void resize_impl(String str) {
    STANDARD_ASSERTS(str);
    ASSERT_CRASH(str->fixed_capacity == false, "Cannot resize a fixed size str.");

    size_t new_capacity = str->capacity + 1;
    do {
        if (str->length >= new_capacity) {
            new_capacity *= 2;
        } else if (str->length < new_capacity / 4) {
            new_capacity /= 2;
        } else {
            return;
        }
    } while (new_capacity < str->length);

    unsigned char *new_content = realloc(str->content, new_capacity * sizeof(char));
    ASSERT_CRASH(new_content != NULL, "Out of memory.");
    memset(new_content + str->length, 0, new_capacity - str->length);

    str->capacity = new_capacity - 1;
    str->content = new_content;
}

static void safe_resize_impl(String str) {
    STANDARD_ASSERTS(str);
    if (str->fixed_capacity == true) {
        /* Only becomes an issue when we actually need to resize
         * A fixed size str, so we only error in that case.
         * All other cases the string is just not resized. Cause you
         * know, it's fixed size.
         */
        ASSERT_CRASH(str->length <= str->capacity,
            "Cannot resize a fixed size string.");
    } else {
        resize_impl(str);
    }
}

String String_new(size_t fixed_size) {
    String new = calloc(1, sizeof(_String));
    ASSERT_CRASH(new != NULL, "Out of memory.");

    new->content = calloc(sizeof(char), fixed_size + 1);
    ASSERT_CRASH(new->content != NULL, "Out of memory.");

    new->cookie = COOKIE;
    new->fixed_capacity = (bool)(fixed_size > 0);
    new->capacity = fixed_size;
    new->length = 0;
    new->rwlock = calloc(1, sizeof(pthread_rwlock_t));
    ASSERT_CRASH(new->rwlock != NULL, "Out of memory.");
    pthread_rwlock_init(new->rwlock, NULL);

    return new;
}

String String_from_literal(const char *literal, size_t fixed_size) {
    size_t literal_len = strlen(literal);
    if (fixed_size > 0) {
        ASSERT_CRASH(literal_len <= fixed_size, "Given string literal too large for given size.");
    }
    String new = String_new(fixed_size);

    pthread_rwlock_wrlock(new->rwlock);
    STANDARD_ASSERTS(new);
    new->length = literal_len;
    safe_resize_impl(new);
    memcpy(new->content, literal, literal_len);
    pthread_rwlock_unlock(new->rwlock);

    return new;
}

void String_free(String *str) {
    pthread_rwlock_wrlock((*str)->rwlock);

    STANDARD_ASSERTS(*str);
    memset((*str)->content, 0, (*str)->capacity + 1);
    free((*str)->content);

    (*str)->cookie = 0;

    pthread_rwlock_unlock((*str)->rwlock);
    pthread_rwlock_destroy((*str)->rwlock);
    free((*str)->rwlock);
    (*str)->rwlock = NULL;

    free(*str);
    *str = NULL;
}

void String_append_impl(String str, unsigned char ch) {
    STANDARD_ASSERTS(str);
    str->length++;
    safe_resize_impl(str);
    str->content[str->length - 1] = ch;
}

void String_append(String str, unsigned char ch) {
    pthread_rwlock_wrlock(str->rwlock);
    STANDARD_ASSERTS(str);
    String_append_impl(str, ch);
    pthread_rwlock_unlock(str->rwlock);
}

String String_copy_impl(Const_String str) {
    STANDARD_ASSERTS(str);
    String copy;
    if (str->fixed_capacity) {
        copy = String_new(str->capacity);
    } else {
        copy = String_new(0);
    }
    pthread_rwlock_wrlock(copy->rwlock);
    STANDARD_ASSERTS(copy);
    ASSERT_CRASH(copy != str, "String copy cannot be the same object as the original.");

    copy->length = str->length;
    safe_resize_impl(copy);
    memcpy(copy->content, str->content, str->length);
    ASSERT_CRASH(copy->length == str->length, "Copy length does not match original length.");
    pthread_rwlock_unlock(copy->rwlock);
    return copy;
}

String String_copy(Const_String str) {
    pthread_rwlock_wrlock(str->rwlock);
    STANDARD_ASSERTS(str);
    Const_String copy = String_copy_impl(str);
    pthread_rwlock_unlock(str->rwlock);
    return (String)copy;
}

void String_insert_impl(String str, size_t index, unsigned char ch) {
    STANDARD_ASSERTS(str);
    str->length++;
    safe_resize_impl(str);
    ASSERT_CRASH(index < str->length, "Cannot insert outside the String.");
    if (str->length == 0 || (str->length == 1 && index == 1)) {
        String_append_impl(str, ch);
        return;
    }
    memmove(str->content + index + 1, str->content + index, str->length - index - 1);
    str->content[index] = ch;
}

void String_insert(String str, size_t index, unsigned char ch) {
    pthread_rwlock_wrlock(str->rwlock);
    STANDARD_ASSERTS(str);
    String_insert_impl(str, index, ch);
    pthread_rwlock_unlock(str->rwlock);
}

void String_prepend_impl(String str, unsigned char ch) {
    STANDARD_ASSERTS(str);
    String_insert_impl(str, 0, ch);
}

void String_prepend(String str, unsigned char ch) {
    pthread_rwlock_wrlock(str->rwlock);
    STANDARD_ASSERTS(str);
    String_prepend_impl(str, ch);
    pthread_rwlock_unlock(str->rwlock);
}

String String_capitalise_impl(Const_String str) {
    STANDARD_ASSERTS(str);
    String new = String_copy_impl(str);
    pthread_rwlock_wrlock(new->rwlock);
    new->content[0] = (unsigned char)toupper(new->content[0]);
    for (size_t i = 1; i < str->length; i++) {
        new->content[i] = (unsigned char)tolower(new->content[i]);
    }
    pthread_rwlock_unlock(new->rwlock);
    return new;
}

String String_capitalise(Const_String str) {
    pthread_rwlock_wrlock(str->rwlock);
    STANDARD_ASSERTS(str);
    Const_String new = String_capitalise_impl(str);
    pthread_rwlock_unlock(str->rwlock);
    return (String)new;
}

void String_capitalise_mut_impl(String str) {
    STANDARD_ASSERTS(str);
    str->content[0] = (unsigned char)toupper(str->content[0]);
    for (size_t i = 1; i < str->length; i++) {
        str->content[i] = (unsigned char)tolower(str->content[i]);
    }
}

void String_capitalise_mut(String str) {
    pthread_rwlock_wrlock(str->rwlock);
    STANDARD_ASSERTS(str);
    String_capitalise_mut_impl(str);
    pthread_rwlock_unlock(str->rwlock);
}

String String_lower_impl(Const_String str) {
    STANDARD_ASSERTS(str);
    String new = String_copy_impl(str);
    pthread_rwlock_wrlock(new->rwlock);
    for (size_t i = 0; i < str->length; i++) {
        new->content[i] = (unsigned char)tolower(new->content[i]);
    }
    pthread_rwlock_unlock(new->rwlock);
    return new;
}

String String_lower(Const_String str) {
    pthread_rwlock_wrlock(str->rwlock);
    STANDARD_ASSERTS(str);
    Const_String new = String_lower_impl(str);
    pthread_rwlock_unlock(str->rwlock);
    return (String)new;
}

void String_lower_mut_impl(String str) {
    STANDARD_ASSERTS(str);
    for (size_t i = 0; i < str->length; i++) {
        str->content[i] = (unsigned char)tolower(str->content[i]);
    }
}

void String_lower_mut(String str) {
    pthread_rwlock_wrlock(str->rwlock);
    STANDARD_ASSERTS(str);
    String_lower_mut_impl(str);
    pthread_rwlock_unlock(str->rwlock);
}

String String_upper_impl(Const_String str) {
    STANDARD_ASSERTS(str);
    String new = String_copy_impl(str);
    pthread_rwlock_wrlock(new->rwlock);
    for (size_t i = 0; i < str->length; i++) {
        new->content[i] = (unsigned char)toupper(new->content[i]);
    }
    pthread_rwlock_unlock(new->rwlock);
    return new;
}

String String_upper(Const_String str) {
    pthread_rwlock_wrlock(str->rwlock);
    STANDARD_ASSERTS(str);
    Const_String new = String_upper_impl(str);
    pthread_rwlock_unlock(str->rwlock);
    return (String)new;
}

void String_upper_mut_impl(String str) {
    STANDARD_ASSERTS(str);
    for (size_t i = 0; i < str->length; i++) {
        str->content[i] = (unsigned char)toupper(str->content[i]);
    }
}

void String_upper_mut(String str) {
    pthread_rwlock_wrlock(str->rwlock);
    STANDARD_ASSERTS(str);
    String_upper_mut_impl(str);
    pthread_rwlock_unlock(str->rwlock);
}

size_t String_count_impl(Const_String str, const char * sub_str) {
    STANDARD_ASSERTS(str);
    size_t len = strlen(sub_str);
    ASSERT_CRASH(len > 0, "Cannot search for the empty string.");
    size_t count = 0;
    char *addr = (char *)str->content;

    while ((addr = strstr(addr, sub_str))) {
        if (strncmp(addr, sub_str, len) == 0) {
            count++;
        }
        addr++;
    }

    return count;
}

size_t String_count(Const_String str, const char * sub_str) {
    pthread_rwlock_wrlock(str->rwlock);
    STANDARD_ASSERTS(str);
    const size_t count = String_count_impl(str, sub_str);
    pthread_rwlock_unlock(str->rwlock);
    return count;
}

ssize_t String_find_impl(Const_String str, const char * sub_str) {
    STANDARD_ASSERTS(str);
    ASSERT_CRASH(strlen(sub_str) > 0, "Cannot find the empty string.");
    ssize_t index = -1;

    const char *addr = strstr((char *)str->content, sub_str);
    if (addr == NULL) {
        return index;
    }
    index = addr - (char *)str->content;
    return index;
}

ssize_t String_find(Const_String str, const char * sub_str) {
    pthread_rwlock_wrlock(str->rwlock);
    STANDARD_ASSERTS(str);
    const ssize_t index = String_find_impl(str, sub_str);
    pthread_rwlock_unlock(str->rwlock);
    return index;
}

bool String_startswith_impl(Const_String str, const char * sub_str) {
    STANDARD_ASSERTS(str);
    size_t len = strlen(sub_str);
    ASSERT_CRASH(len > 0, "Cannot find the empty string.");

    return strncmp((char *)str->content, sub_str, len) == 0 ? true : false;
}

bool String_startswith(Const_String str, const char * sub_str) {
    pthread_rwlock_wrlock(str->rwlock);
    STANDARD_ASSERTS(str);
    const bool result = String_startswith_impl(str, sub_str);
    pthread_rwlock_unlock(str->rwlock);
    return result;
}

bool String_endswith_impl(Const_String str, const char * sub_str) {
    STANDARD_ASSERTS(str);
    size_t len = strlen(sub_str);
    ASSERT_CRASH(len > 0, "Cannot find the empty string.");

    return strncmp((char *)(str->content + (str->length - len)), sub_str, len) == 0 ? true : false;
}

bool String_endswith(Const_String str, const char * sub_str) {
    pthread_rwlock_wrlock(str->rwlock);
    STANDARD_ASSERTS(str);
    const bool result = String_endswith_impl(str, sub_str);
    pthread_rwlock_unlock(str->rwlock);
    return result;
}

String String_swapcase_impl(Const_String str) {
    STANDARD_ASSERTS(str);
    String new = String_copy_impl(str);
    pthread_rwlock_wrlock(new->rwlock);
    for (size_t i = 0; i < str->length; i++) {
        if (isupper(new->content[i])) {
            new->content[i] = (unsigned char)tolower(new->content[i]);
        } else {
            new->content[i] = (unsigned char)toupper(new->content[i]);
        }
    }
    pthread_rwlock_unlock(new->rwlock);
    return new;
}

String String_swapcase(Const_String str) {
    pthread_rwlock_wrlock(str->rwlock);
    STANDARD_ASSERTS(str);
    String new = String_swapcase_impl(str);
    pthread_rwlock_unlock(str->rwlock);

    return new;
}

void String_swapcase_mut_impl(String str) {
    STANDARD_ASSERTS(str);
    for (size_t i = 0; i < str->length; i++) {
        if (isupper(str->content[i])) {
            str->content[i] = (unsigned char)tolower(str->content[i]);
        } else {
            str->content[i] = (unsigned char)toupper(str->content[i]);
        }
    }
}

void String_swapcase_mut(String str) {
    pthread_rwlock_wrlock(str->rwlock);
    STANDARD_ASSERTS(str);
    String_swapcase_mut_impl(str);
    pthread_rwlock_unlock(str->rwlock);
}

String String_title_impl(Const_String str) {
    STANDARD_ASSERTS(str);
    String new = String_copy_impl(str);
    pthread_rwlock_wrlock(new->rwlock);
    bool wasSpace = false;
    for (size_t i = 0; i < str->length; i++) {
        if (wasSpace) {
            new->content[i] = (unsigned char)toupper(new->content[i]);
            wasSpace = false;
        }
        if (new->content[i] == ' ') {
            wasSpace = true;
        }
    }
    pthread_rwlock_unlock(new->rwlock);
    return new;
}

String String_title(Const_String str) {
    pthread_rwlock_wrlock(str->rwlock);
    STANDARD_ASSERTS(str);
    String new = String_title_impl(str);
    pthread_rwlock_unlock(str->rwlock);
    return new;
}

void String_title_mut_impl(String str) {
    STANDARD_ASSERTS(str);
    bool wasSpace = false;
    for (size_t i = 0; i < str->length; i++) {
        if (wasSpace) {
            str->content[i] = (unsigned char)toupper(str->content[i]);
            wasSpace = false;
        }
        if (str->content[i] == ' ') {
            wasSpace = true;
        }
    }
}

void String_title_mut(String str) {
    pthread_rwlock_wrlock(str->rwlock);
    STANDARD_ASSERTS(str);
    String_title_mut_impl(str);
    pthread_rwlock_unlock(str->rwlock);
}

char *String_data_impl(Const_String str) {
    STANDARD_ASSERTS(str);
    char *data = calloc(str->length + 1, sizeof(char));
    memcpy(data, str->content, str->length);
    return data;
}

char *String_data(Const_String str) {
    pthread_rwlock_rdlock(str->rwlock);
    STANDARD_ASSERTS(str);
    char *data = String_data_impl(str);
    pthread_rwlock_unlock(str->rwlock);
    return data;
}

String String_replace_impl(Const_String str, const char *old_pat, const char *new_pat) {
    STANDARD_ASSERTS(str);
    String new = String_copy_impl(str);
    pthread_rwlock_wrlock(new->rwlock);
    char *sub_pattern;
    while ((sub_pattern = strstr((char *)new->content, old_pat)) != NULL) {
        char *post_pattern = sub_pattern + strlen(old_pat);
        char *move_post_pattern = sub_pattern + strlen(new_pat);

        const long long delta_len = (long long)strlen(new_pat) - (long long)strlen(old_pat);
        new->length += delta_len;
        safe_resize_impl(new);

        memmove(move_post_pattern, post_pattern, strlen(post_pattern));
        memmove(sub_pattern, new_pat, strlen(new_pat));
        memset(new->content + new->length, '\0', new->capacity - new->length);
    }
    pthread_rwlock_unlock(new->rwlock);
    return new;
}

String String_replace(Const_String str, const char *old_pat, const char *new_pat) {
    pthread_rwlock_wrlock(str->rwlock);
    STANDARD_ASSERTS(str);
    String new = String_replace_impl(str, old_pat, new_pat);
    pthread_rwlock_unlock(str->rwlock);
    return new;
}

void String_replace_mut_impl(String str, const char *old_pat, const char *new_pat) {
    STANDARD_ASSERTS(str);
    char *sub_pattern;
    while ((sub_pattern = strstr((char *)str->content, old_pat)) != NULL) {
        char *post_pattern = sub_pattern + strlen(old_pat);
        char *move_post_pattern = sub_pattern + strlen(new_pat);

        long long delta_len = (long long)strlen(new_pat) - (long long)strlen(old_pat);
        str->length += delta_len;
        safe_resize_impl(str);

        memmove(move_post_pattern, post_pattern, strlen(post_pattern));
        memmove(sub_pattern, new_pat, strlen(new_pat));
        memset(str->content + str->length, '\0', str->capacity - str->length);
    }
}

void String_replace_mut(String str, const char *old_pat, const char *new_pat) {
    pthread_rwlock_wrlock(str->rwlock);
    STANDARD_ASSERTS(str);
    String_replace_mut_impl(str, old_pat, new_pat);
    pthread_rwlock_unlock(str->rwlock);
}

