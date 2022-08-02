#if !defined(__JSONARR_INCLUDED__)
#define __JSONARR_INCLUDED__
#include <stddef.h>


/** Construct a new JsonArray and return its pointer, or NULL. */
JsonArray *jsonarr_construct(size_t capacity);

/** Make a shallow copy of array, with inclusive *start* and exclusive *end*.
 *
 * The copy's initial capacity will be set to the source's current length.
 * Returns NULL if allocation fails.
 * If *start* is bigger than *end*, the result is empty array.
 * If *start* or *end* goes out of boundary, they're set to be within it.
 */
JsonArray *jsonarr_slice(JsonArray *array, size_t start, size_t end);

/** Destruct the array. */
void jsonarr_destruct();

/** Resize the array to fit its current length. */
bool jsonarr_fit(JsonArray *array);

/** Get item pointer at given index. Out of bound is unrecoverable error. */
JsonValue *jsonarr_getitem(JsonArray *array, size_t index);

/** Set item at index. */
void jsonarr_setitem(JsonArray *array, size_t index, JsonValue *item);

/** Delete item at index. May return false if shrinking fails. */
bool jsonarr_delitem(JsonArray *array, size_t index);

/** Clear the array and zerofill. */
void jsonarr_clear(JsonArray *array);

/** Append item at the end of the array. Return false on failure. */
bool jsonarr_append(JsonArray *array, JsonValue *item);

/**Insert item into array at given index, returning  false on failure.
 *
 * Item is inserted such that it would be later accessed by *index*.
 */
bool jsonarr_insert(JsonArray *array, size_t index, JsonValue *item);

/** Pop from the end of array. May return NULL if shrinking fails. */
JsonValue *jsonarr_pop(JsonArray *array);

/** Find first index that matches the item according to jsonval_equal().
 *
 * The items are compared starting from the first.
 * If no match is found, returns SIZE_MAX.
 */
size_t jsonarr_index(JsonArray *array, JsonValue *item, size_t stop);

/** Return an iterator of the array, or NULL if allocation fails. */
JsonArrayIterator *jsonarr_iter(JsonArray *array);

/** Advance the iterator. Iterator will be freed and return false at the end. */
bool jsonarr_next(JsonArrayIterator *iter);


#endif
