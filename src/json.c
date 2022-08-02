#include <stdio.h>
#include <string.h>

#include "json.h"
#include "jsonarr.h"
#include "jsonobj.h"


/** Test equqlity between JsonValue.
 * Arrays and Objects are recursively tested.
 */
bool jsonval_equal(JsonValue *a, JsonValue *b) {
    bool equal;
    int numeric = 0;
    JsonValueType type_a = a->type;
    JsonValueType type_b = b->type;
    JsonValue *val_a;
    JsonValue *val_b;

    if (type_a != type_b) {
        return false;
    }

    switch (type_a) {
        case JSON_NULL:
            equal = true;
            break;
        case JSON_BOOL:
            equal = !!a->value.as_bool == !!b->value.as_bool;
            break;
        case JSON_NUMBER:
            equal = a->value.as_num == b->value.as_num;
            break;
        case JSON_STRING:
            equal = strcmp(a->value.as_str, b->value.as_str) == 0;
            break;
        case JSON_ARRAY:
            JsonArray *arr_a = a->value.as_arr;
            JsonArray *arr_b = b->value.as_arr;
            if (arr_a->len != arr_b->len) {
                equal = false;
            } else {
                // Need to check each element.
                equal = true;
                size_t len = arr_a->len;
                for (size_t i = 0; i < len; i++) {
                    val_a = jsonarr_getitem(arr_a, i);
                    val_b = jsonarr_getitem(arr_b, i);
                    if (!jsonval_equal(val_a, val_b)) {
                        equal = false;
                        break;
                    }
                }
            }
            break;
        case JSON_OBJECT:
            JsonObject *obj_a = a->value.as_obj;
            JsonObject *obj_b = b->value.as_obj;
            JsonObjectIterator *iter;
            if (obj_a->len != obj_b->len) {
                equal = false;
            } else {
                if (!(iter = jsonobj_iter(obj_a))) {
                    equal = false;
                } else {
                    equal = true;
                    while (jsonobj_next(iter)) {
                        val_a = iter->value;
                        if (!jsonobj_contains(obj_b, iter->key)) {
                            equal = false;
                            break;
                        }
                        val_b = jsonobj_getitem(obj_b, iter->key);
                        if (!jsonval_equal(val_a, val_b)) {
                            equal = false;
                            break;
                        }
                    }
                }
            }
            break;
    }
    return equal;
}


void jsonval_fprint(FILE *stream, JsonValue *item) {
    switch (item->type) {
        case JSON_NULL:
            fprintf(stream, "null");
            break;
        case JSON_BOOL:
            fprintf(stream, "%s", (item->value.as_bool) ? "true" : "false");
            break;
        case JSON_NUMBER:
            fprintf(stream, "%g", item->value.as_num);
            break;
        case JSON_STRING:
            // todo: escaping and quoting
            fprintf(stream, "%s", item->value.as_str);
            break;
        case JSON_ARRAY:
            fprintf(stream, "[");
            break;
        case JSON_OBJECT:
            break;
    }
}
