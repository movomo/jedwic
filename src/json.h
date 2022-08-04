#ifndef __JSON_H__
#define __JSON_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>


typedef enum JsonValueType {
    JSON_NULL,
    JSON_BOOL,
    JSON_NUMBER,
    JSON_STRING,
    JSON_ARRAY,
    JSON_OBJECT
} JsonValueType;


typedef struct JsonValue JsonValue;
typedef struct JsonArray JsonArray;
typedef struct JsonObjectEntry JsonObjectEntry;
typedef struct JsonObject JsonObject;


struct JsonValue {
    JsonValueType type;
    union _JsonValue {
        bool as_bool;
        double as_num;
        char *as_str;
        JsonArray *as_arr;
        JsonObject *as_obj;
    } value;
};


struct JsonArray {
    size_t len;
    size_t _cap;
    JsonValue *_data;
};

typedef struct JsonArrayIterator {
    size_t index;
    JsonValue *value;
    JsonArray *_arr;
} JsonArrayIterator;


// 64-bit modulo is known to be slow yet.
typedef uint_fast64_t JsonObjectKeyHash;
typedef JsonObjectKeyHash (*JsonObjectHashFunction)(void *data);

struct JsonObjectEntry {
    char *key;
    JsonObjectKeyHash _hash;
    JsonValue value;
};

typedef struct _JsonObjectBucket {
    JsonObjectEntry entry;
    struct _JsonObjectBucket *next;
} _JsonObjectBucket;

struct JsonObject {
    size_t len;
    size_t _cap;
    JsonObjectHashFunction _hasher;
    _JsonObjectBucket **_data;
};

typedef struct JsonObjectIterator {
    char *key;
    JsonValue *value;
    size_t index;
    size_t _index;
    JsonObject *_obj;
    _JsonObjectBucket *_bucket;
} JsonObjectIterator;


/** Test equqlity between JsonValue.
 * Arrays and Objects are recursively tested.
 */
bool jsonval_equal(JsonValue *a, JsonValue *b);

/** Decode JSON string and return as a pointer to JsonValue.
 *
 * Set *error* value to true when parsing failed.
 * *text* is not destroyed after decoding.
 */
JsonValue json_sdecode(char *text, bool *error);

void json_fencode(FILE *stream, JsonValue *item);

/** Implements 32-bit FNV-1a hash algorithm. Expects string as input.*/
JsonObjectKeyHash json_default_hasher(void *data);


#endif
