#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "json.h"
#include "jsonobj.h"


#define JSONOBJ_FNV_PRIME_32        0x01000193
#define JSONOBJ_FNV_OFFSET_BASIS_32 0x811c9dc5
#define JSONOBJ_FNV_PRIME_64        0x00000100000001b3
#define JSONOBJ_FNV_OFFSET_BASIS_64 0xcbf29ce484222325
#define JSONOBJ_CAPACITY_STEPS      29
#define JSONOBJ_GROW_THRESHOLD      2 / 3
#define JSONOBJ_SHRINK_THRESHOLD    4


static const size_t _JSONOBJ_CAPS[JSONOBJ_CAPACITY_STEPS] = {
    7,
    13,
    29,
    53,
    97,
    193,
    389,
    769,
    1543,
    3079,
    6151,
    12289,
    24593,
    49157,
    98317,
    196613,
    393241,
    786433,
    1572869,
    3145739,
    6291469,
    12582917,
    25165843,
    50331653,
    100663319,
    201326611,
    402653189,
    805306457,
    1610612741
};


/** Implements 32-bit FNV-1a hash algorithm. Expects string as input.*/
JsonObjectKeyHash json_default_hasher(void *data) {
    unsigned char *bytes = data;
    JsonObjectKeyHash b;
    // JsonObjectKeyHash hash = JSONOBJ_FNV_OFFSET_BASIS_32;
    JsonObjectKeyHash hash = JSONOBJ_FNV_OFFSET_BASIS_64;
    while ((b = *bytes++) != 0) {
        // hash = (hash ^ b) * JSONOBJ_FNV_PRIME_32;
        hash = (hash ^ b) * JSONOBJ_FNV_PRIME_64;
    }
    return hash;
}


static void _jsonobj_print_obj(JsonObject *object) {
    JsonObjectIterator *iter = jsonobj_iter(object);

    printf(
        "JsonObject<%p>(len=%llu, _cap=%llu, _data<%p>=",
        object,
        object->len,
        object->_cap,
        object->_data
    );

    printf("{");
    while (jsonobj_next(iter)) {
        if (!(iter->index == 0)) {
            printf(", ");
        }
        printf("\"%s\": %g", iter->key, iter->value->value.as_num);
    }
    printf("})\n");
}

static void _jsonobj_print_entry(JsonObjectEntry *entry) {
    printf(
        "JsonObjectEntry<%p>(key=%s, _hash=%llu, &value=%p)\n",
        entry,
        entry->key,
        entry->_hash,
        &entry->value
    );
}


static void _jsonobj_error_key(char *key) {
    fprintf(stderr, "JsonObject: key(%s) not found\n", key);
    abort();
}

static bool _jsonobj_resize(JsonObject *object, unsigned int index) {
    size_t new_cap = _JSONOBJ_CAPS[index];
    size_t old_idx;
    size_t new_idx;
    _JsonObjectBucket **new_data;
    _JsonObjectBucket *bucket;
    _JsonObjectBucket *prev;
    _JsonObjectBucket *new_place;

    new_data = calloc(new_cap, sizeof (_JsonObjectBucket *));
    if (!new_data) {
        return false;
    }

    for (old_idx = 0; old_idx < object->_cap; old_idx++) {
        bucket = object->_data[old_idx];
        while (bucket) {
            prev = bucket;
            bucket = bucket->next;
            prev->next = NULL;

            new_idx = prev->entry._hash % new_cap;

            new_place = new_data[new_idx];
            if (!new_place) {
                new_data[new_idx] = prev;
            } else {
                while (new_place->next) {
                    new_place = new_place->next;
                }
                new_place->next = prev;
            }
        }
    }

    free(object->_data);
    object->_cap = new_cap;
    object->_data = new_data;
    return true;
}

static inline bool _jsonobj_grow(JsonObject *object) {
    if (object->len <= object->_cap * JSONOBJ_GROW_THRESHOLD) {
        return true;
    }

    // The step array is small enough that bsearch is probably not worth it.
    for (unsigned int i = 0; i < JSONOBJ_CAPACITY_STEPS; i++) {
        if (_JSONOBJ_CAPS[i] > object->_cap) {
            return _jsonobj_resize(object, i);
        }
    }
    return false;
}

static inline bool _jsonobj_shrink(JsonObject *object) {
    if (object->len >= object->_cap / JSONOBJ_SHRINK_THRESHOLD) {
        return true;
    }

    for (unsigned int i = JSONOBJ_CAPACITY_STEPS - 1; i-- > 0; ) {
        if (_JSONOBJ_CAPS[i] < object->_cap) {
            return _jsonobj_resize(object, i);
        }
    }
    return false;
}

static void _jsonobj_destruct_bucket(_JsonObjectBucket *bucket) {
    if (bucket->next) {
        _jsonobj_destruct_bucket(bucket->next);
    }
    free(bucket->entry.key);
    free(bucket);
}

static inline _JsonObjectBucket *_jsonobj_contains(JsonObject *object, char *key) {
    JsonObjectKeyHash hash = object->_hasher(key);
    size_t index = hash % object->_cap;
    _JsonObjectBucket *bucket = object->_data[index];

    if (!bucket) {
        return NULL;
    } else {
        do {
            if (bucket->entry._hash == hash
                    && strcmp(bucket->entry.key, key) == 0) {
                return bucket;
            }
        } while (bucket = bucket->next);
    }
    return NULL;
}


/** Construct a JsonObject with at least *min_capacity*.
 *
 * If *min_capacity* is not SIZE_MAX, this will try to allocate at least that
 * amount of capacity.
 * If memory allocation faile at any step, this will return NULL.
 */
JsonObject *jsonobj_construct(
    JsonObjectHashFunction hasher, size_t min_capacity
) {
    JsonObject *object = malloc(sizeof(JsonObject));
    if (!object) {
        return NULL;
    }

    size_t cap = 0;

    if (min_capacity == SIZE_MAX) {
        cap = _JSONOBJ_CAPS[0];
    } else {
        for (unsigned int i = 0; i < JSONOBJ_CAPACITY_STEPS; i++) {
            if (_JSONOBJ_CAPS[i] >= min_capacity) {
                cap = _JSONOBJ_CAPS[i];
                break;
            }
        }
        if (cap == 0) {
            // We never imagined tables this big. Let us run like hell.
            free(object);
            return NULL;
        }
    }
    object->len = 0;
    object->_hasher = hasher;
    object->_cap = cap;
    object->_data = calloc(object->_cap, sizeof (_JsonObjectBucket *));
    if (!object->_data) {
        free(object);
        return NULL;
    }
    return object;
}

/** Destruct object. */
void jsonobj_destruct(JsonObject *object) {
    jsonobj_clear(object);
    free(object->_data);
    free(object);
}

/** Get the item associated with *key*. Querying non-existent key is error. */
JsonValue *jsonobj_getitem(JsonObject *object, char *key) {
    _JsonObjectBucket *bucket = _jsonobj_contains(object, key);
    if (!bucket) {
        _jsonobj_error_key(key);
    }
    return &bucket->entry.value;
}

/** Associate *key* with *value*. It can fail and return false. */
bool jsonobj_setitem(JsonObject *object, char *key, JsonValue *value) {
    _JsonObjectBucket *bucket = _jsonobj_contains(object, key);
    if (bucket) {
        bucket->entry.value = *value;
        return true;
    }

    if (!_jsonobj_grow(object)) {
        return false;
    }

    bucket = calloc(1, sizeof(_JsonObjectBucket));
    if (!bucket) {
        return false;
    }

    // Keep a copy because key should not change.
    JsonObjectEntry *entry = &bucket->entry;
    entry->key = malloc((strlen(key) + 1) * sizeof (char));
    if (!entry->key) {
        free(bucket);
        return false;
    }
    strcpy(entry->key, key);

    JsonObjectKeyHash hash = object->_hasher(key);
    size_t index = hash % object->_cap;
    entry->_hash = hash;

    entry->value = *value;

    // Find a place for this entry to sit.
    _JsonObjectBucket *place = object->_data[index];
    if (!place) {
        object->_data[index] = bucket;
    } else {
        while (place->next) {
            place = place->next;
        }
        place->next = bucket;
    }

    object->len++;
    return true;
}

/** Delete *key* and associated *value*. It can fail and return false. */
bool jsonobj_delitem(JsonObject *object, char *key) {
    JsonObjectKeyHash hash = object->_hasher(key);
    size_t index = hash % object->_cap;
    _JsonObjectBucket *bucket = object->_data[index];
    _JsonObjectBucket *prev;

    if (!bucket) {
        _jsonobj_error_key(key);
    }
    if (!_jsonobj_shrink(object)) {
        return false;
    }

    // Need to recalculate index in case it actually shrank.
    index = hash % object->_cap;
    bucket = object->_data[index];
    prev = bucket;
    if (!bucket->next) {
        if (bucket->entry._hash == hash
                && strcmp(bucket->entry.key, key) == 0) {
            object->_data[index] = NULL;
            object->len--;
            // Free the copy of key we previously had.
            free(bucket->entry.key);
            free(bucket);
            return true;
        } else {
            _jsonobj_error_key(key);
        }
    } else {
        do {
            if (bucket->entry._hash == hash
                    && strcmp(bucket->entry.key, key) == 0) {
                prev->next = bucket->next;
                object->len--;
                free(bucket->entry.key);
                free(bucket);
                return true;
            }
            prev = bucket;
            bucket = bucket->next;
        } while (bucket);
    }
    return false;
}

/** Report if the object has item associated with *key*. */
bool jsonobj_contains(JsonObject *object, char *key) {
    return _jsonobj_contains(object, key) != NULL;
}

/** Clear object and free all of its resources. */
void jsonobj_clear(JsonObject *object) {
    size_t idx;
    _JsonObjectBucket *bucket;

    for (idx = 0; idx < object->_cap; idx++) {
        bucket = object->_data[idx];
        if (bucket) {
            _jsonobj_destruct_bucket(bucket);
        }
    }
}

/** Return an iterator to the object. Return NULL if allocation fails. */
JsonObjectIterator *jsonobj_iter(JsonObject *object) {
    JsonObjectIterator *iter = malloc(sizeof (JsonObjectIterator));
    if (!iter) {
        return NULL;
    }

    iter->key = NULL;
    iter->value = NULL;
    iter->index = SIZE_MAX;
    iter->_index = SIZE_MAX;
    iter->_obj = object;
    iter->_bucket = NULL;
    return iter;
}

/** Advance the iterator and report existence of next entry. */
bool jsonobj_next(JsonObjectIterator *iter) {
    JsonObject *object = iter->_obj;
    _JsonObjectBucket *bucket;

    iter->index++;

    bucket = iter->_bucket;
    if (bucket && bucket->next) {
        bucket = bucket->next;
        iter->_bucket = bucket;
        iter->key = bucket->entry.key;
        iter->value = &bucket->entry.value;
        return true;
    }

    // Find whatever outermost bucket.
    while (++iter->_index < object->_cap) {
        bucket = object->_data[iter->_index];
        if (bucket) {
            iter->_bucket = bucket;
            iter->key = bucket->entry.key;
            iter->value = &bucket->entry.value;
            return true;
        }
    }

    free(iter);
    return false;
}
