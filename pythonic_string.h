//
// Created by cheerfulmoss on 02/03/25.
//

#ifndef SHIT_STRING_H
#define SHIT_STRING_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

typedef struct {
    unsigned char *content;
    size_t length;
    uint16_t cookie;
    size_t capacity;
    bool fixed_capacity;
    pthread_rwlock_t *rwlock;
} _String;

typedef _String *String;
typedef const _String * const Const_String;

String String_new(size_t);
void String_free(String *);
String String_from_literal(const char *, size_t);
void String_append(String, unsigned char);
String String_copy(Const_String);
void String_prepend(String, unsigned char);
void String_insert(String, size_t, unsigned char);
String String_capitalise(Const_String);
void String_capitalise_mut(String);
String String_lower(Const_String);
void String_lower_mut(String);
String String_upper(Const_String);
void String_upper_mut(String);
size_t String_count(Const_String, const char *);
ssize_t String_find(Const_String, const char *);
bool String_endswith(Const_String, const char *);
bool String_startswith(Const_String, const char *);
String String_swapcase(Const_String);
void String_swapcase_mut(String);
String String_title(Const_String);
void String_title_mut(String);
char *String_data(Const_String);
String String_replace(Const_String, const char *, const char *);
void String_replace_mut(String, const char *, const char *);


#endif //SHIT_STRING_H
