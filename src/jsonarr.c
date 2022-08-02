#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "json.h"
#include "jsonarr.h"


#define JSONARRAY_INITIAL_CAPACITY  4
#define JSONARRAY_GROW_FACTOR       1.5
#define JSONARRAY_SHRINK_FACTOR     2
#define JSONARRAY_SHRINK_THRESHOLD  3


static inline void _jsonarr_test_index(size_t len, size_t idx) {
    if (idx >= len) {
        fprintf(
            stderr, "JsonArray: index(%u) out of bound(%u)\n", idx, len
        );
        abort();
    }
}

static bool _jsonarr_resize(JsonArray *array, size_t new_cap) {
    JsonValue *new_data = calloc(new_cap, sizeof (JsonValue));
    if (!new_data) {
        return false;
    }

    memcpy(new_data, array->_data, array->len * sizeof (JsonValue));
    free(array->_data);
    array->_data = new_data;
    array->_cap = new_cap;
    return true;
}

static inline bool _jsonarr_grow(JsonArray *array) {
    if (array->len < array->_cap) {
        return true;
    }
    size_t new_cap = array->_cap * JSONARRAY_GROW_FACTOR;
    return _jsonarr_resize(array, new_cap);
}

static inline bool _jsonarr_shrink(JsonArray *array) {
    if (array->len > array->_cap / JSONARRAY_SHRINK_THRESHOLD) {
        return true;
    }
    size_t new_cap = array->_cap / JSONARRAY_SHRINK_FACTOR;
    if (new_cap < array->len) {
        new_cap = array->len;
    }
    return _jsonarr_resize(array, new_cap);
}


/** Construct a new JsonArray and return its pointer, or NULL. */
JsonArray *jsonarr_construct(size_t capacity) {
    if (capacity == SIZE_MAX) {
        capacity = JSONARRAY_INITIAL_CAPACITY;
    }

    JsonArray *array = malloc(sizeof(JsonArray));
    if (!array) {
        return NULL;
    }

    array->_data = calloc(capacity, sizeof(JsonValue));
    if (!array->_data) {
        free(array);
        return NULL;
    }

    array->len = 0;
    array->_cap = capacity;
    return array;
}

/** Make a shallow copy of array, with inclusive *start* and exclusive *end*.
 *
 * The copy's initial capacity will be set to the source's current length.
 * Returns NULL if allocation fails.
 * If *start* is bigger than *end*, the result is empty array.
 * If *start* or *end* goes out of boundary, they're set to be within it.
 */
JsonArray *jsonarr_slice(JsonArray *array, size_t start, size_t end) {
    if (start < 0) {
        start = 0;
    }
    if (end > array->len) {
        end = array->len;
    }

    size_t cap = end - start;
    cap = (cap >= 0) ? cap : 0;
    JsonArray *copy = jsonarr_construct(cap);
    if (!copy) {
        return NULL;
    }

    for (size_t i = start; i < end; i++) {
        jsonarr_append(copy, &array->_data[i]);
    }

    return copy;
}

/** Destruct the array. */
void jsonarr_destruct(JsonArray *array) {
    free(array->_data);
    free(array);
}

/** Resize the array to fit its current length. */
bool jsonarr_fit(JsonArray *array) {
    return _jsonarr_resize(array, array->len);
}

/** Get item pointer at given index. Out of bound is unrecoverable error. */
JsonValue *jsonarr_getitem(JsonArray *array, size_t index) {
    _jsonarr_test_index(array->len, index);
    return &array->_data[index];
}

/** Set item at index. */
void jsonarr_setitem(JsonArray *array, size_t index, JsonValue *value) {
    _jsonarr_test_index(array->len, index);
    array->_data[index] = *value;
}

/** Delete item at index. May return false if shrinking fails. */
bool jsonarr_delitem(JsonArray *array, size_t index) {
    _jsonarr_test_index(array->len, index);
    if (!_jsonarr_shrink(array)) {
        return false;
    };

    size_t len = array->len--;
    JsonValue *data = array->_data;
    for (size_t i = index + 1; i < len; i++) {
        data[i - 1] = data[i];
    }
    return true;
}

/** Clear the array and zerofill. */
void jsonarr_clear(JsonArray *array) {
    // There's primarily one reason to clear an array: to refill it.
    // So we don't bother shrinking it.
    array->len = 0;
    memset(array->_data, 0, array->len * sizeof (JsonValue));
}

/** Append item at the end of the array. Return false on failure. */
bool jsonarr_append(JsonArray *array, JsonValue *item) {
    if (!_jsonarr_grow(array)) {
        return false;
    }

    array->_data[array->len++] = *item;
    return true;
}

/**Insert item into array at given index, returning  false on failure.
 *
 * Item is inserted such that it would be later accessed by *index*.
 */
bool jsonarr_insert(JsonArray *array, size_t index, JsonValue *item) {
    // Index test is special here (+1) because insert allows +1 out of bound.
    _jsonarr_test_index(array->len + 1, index);
    if (!_jsonarr_grow(array)) {
        return false;
    }

    JsonValue *data = array->_data;
    for (size_t i = array->len; i-- > index; ) {
        data[i + 1] = data[i];
    }
    data[index] = *item;
    array->len++;
    return true;
}

/** Pop from the end of array. May return NULL if shrinking fails. */
JsonValue *jsonarr_pop(JsonArray *array) {
    if (!_jsonarr_shrink(array)) {
        return NULL;
    };
    return &array->_data[--array->len];
}

/** Find first index that matches the item according to jsonval_equal().
 *
 * The items are compared starting from the first.
 * If no match is found, returns SIZE_MAX.
 */
size_t jsonarr_index(JsonArray *array, JsonValue *item, size_t stop) {
    size_t len = array->len;
    JsonValue *data = array->_data;
    for (size_t i = 0; i < len && i < stop; i++) {
        JsonValue *other = &data[i];
        if (jsonval_equal(item, other)) {
            return i;
        }
    }
    return SIZE_MAX;
}

/** Return an iterator of the array, or NULL if allocation fails. */
JsonArrayIterator *jsonarr_iter(JsonArray *array) {
    JsonArrayIterator *iter = malloc(sizeof (JsonArrayIterator));
    if (!iter) {
        return NULL;
    }
    iter->index = SIZE_MAX;
    iter->value = NULL;
    iter->_arr = array;
    return iter;
}

/** Advance the iterator. Iterator will be freed and return false at the end. */
bool jsonarr_next(JsonArrayIterator *iter) {
    JsonArray *array = iter->_arr;
    if (++iter->index == array->len) {
        free(iter);
        return false;
    }
    iter->value = &array->_data[iter->index];
    return true;
}
