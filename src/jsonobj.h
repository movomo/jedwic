#ifndef __JSONOBJ_H__
#define __JSONOBJ_H__


/** Construct a JsonObject with at least *min_capacity*.
 *
 * If *min_capacity* is not SIZE_MAX, this will try to allocate at least that
 * amount of capacity.
 * If memory allocation faile at any step, this will return NULL.
 */
JsonObject *jsonobj_construct(
    JsonObjectHashFunction hasher, size_t min_capacity
);

/** Destruct object. */
void jsonobj_destruct(JsonObject *object);

/** Get the item associated with *key*. Querying non-existent key is error. */
JsonValue *jsonobj_getitem(JsonObject *object, char *key);

/** Associate *key* with *value*. It can fail and return false. */
bool jsonobj_setitem(JsonObject *object, char *key, JsonValue *value);

/** Delete *key* and associated *value*. It can fail and return false. */
bool jsonobj_delitem(JsonObject *object, char *key);

/** Report if the object has item associated with *key*. */
bool jsonobj_contains(JsonObject *object, char *key);

/** Clear object and free all of its resources. */
void jsonobj_clear(JsonObject *object);

/** Return an iterator to the object. Return NULL if allocation fails. */
JsonObjectIterator *jsonobj_iter(JsonObject *object);

/** Advance the iterator and report existence of next entry. */
bool jsonobj_next(JsonObjectIterator *iter);


#endif
