#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "token.h"
#include "json.h"
#include "jsonarr.h"
#include "jsonobj.h"
#include "lexer.h"
#include "parser.h"


static JsonValue _json_visit(ASTNode *node);


static JsonValue _json_visit_nullnode(ASTNode *node) {
    JsonValue jsval = { JSON_NULL, .value.as_bool = false };
    return jsval;
}

static JsonValue _json_visit_boolnode(ASTNode *node) {
    JsonValue jsval = { JSON_BOOL };
    jsval.value.as_bool = node->value[0] == 't';
    return jsval;
}

static JsonValue _json_visit_numbernode(ASTNode *node) {
    JsonValue jsval = { JSON_NUMBER };
    jsval.value.as_num = atof(node->value);
    return jsval;
}

static JsonValue _json_visit_stringnode(ASTNode *node) {
    JsonValue jsval = { JSON_STRING };
    jsval.value.as_str = node->value;
    return jsval;
}

static JsonValue _json_visit_arraynode(ASTNode *node) {
    JsonValue jsval = { JSON_ARRAY };
    JsonValue val;
    JsonArray *arr;
    size_t i;

    jsval.value.as_arr = jsonarr_construct(node->len);
    arr = jsval.value.as_arr;
    for (i = 0; i < node->len; i++) {
        val = _json_visit(node->children[i]);
        jsonarr_append(arr, &val);
    }
    return jsval;
}

static JsonValue _json_visit_keynode(ASTNode *node, char **key) {
    JsonValue jsval = _json_visit(node->children[0]);
    *key = node->value;
    return jsval;
}

static JsonValue _json_visit_objectnode(ASTNode *node) {
    JsonValue jsval = { JSON_OBJECT };
    JsonValue val;
    JsonObject *obj;
    size_t i;
    char *key;

    jsval.value.as_obj = jsonobj_construct(json_default_hasher, node->len);
    obj = jsval.value.as_obj;
    for (i = 0; i < node->len; i++) {
        val = _json_visit_keynode(node->children[i], &key);
        jsonobj_setitem(obj, key, &val);
    }
    return jsval;
}

static JsonValue _json_visit(ASTNode *node) {
    JsonValue jsval;

    switch (node->kind) {
        case AST_NULL:
            return _json_visit_nullnode(node);
        case AST_BOOL:
            return _json_visit_boolnode(node);
        case AST_NUMBER:
            return _json_visit_numbernode(node);
        case AST_STRING:
            return _json_visit_stringnode(node);
        case AST_ARRAY:
            return _json_visit_arraynode(node);
        case AST_OBJECT:
            return _json_visit_objectnode(node);
        default:
            assert(0);
            break;
    }
}


static void _json_fencode_string(FILE *stream, char *string) {
    char *chr = string;

    fputc('"', stream);
    while (*chr) {
        switch (*chr) {
            case '"':
                fputc('\\', stream);
                fputc('"', stream);
                break;
            case '\\':
                fputc('\\', stream);
                if (!(*(chr + 1) == 'u')) {
                    fputc('\\', stream);
                }
                break;
            case '\n':
                fputc('\\', stream);
                fputc('n', stream);
                break;
            case '\r':
                fputc('\\', stream);
                fputc('r', stream);
                break;
            case '\t':
                fputc('\\', stream);
                fputc('t', stream);
                break;
            case '\b':
                fputc('\\', stream);
                fputc('b', stream);
                break;
            case '\f':
                fputc('\\', stream);
                fputc('f', stream);
                break;
            default:
                fputc(*chr, stream);
                break;
        }
        chr++;
    }
    fputc('"', stream);
}

static inline void _json_fencode_newline(
    FILE *stream, bool pretty, unsigned int depth
) {
    if (pretty) {
        fputc('\n', stream);

        depth *= 4;
        while (depth--) {
            fputc(' ', stream);
        }
    }
}

static void _json_fencode(
    FILE *stream, JsonValue *item, bool pretty, unsigned int depth
) {
    size_t i;
    size_t len;

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
            _json_fencode_string(stream, item->value.as_str);
            break;
        case JSON_ARRAY:
            JsonArray *arr = item->value.as_arr;

            fputc('[', stream);
            len = arr->len;
            for (i = 0; i < len; i++) {
                if (i) {
                    fputc(',', stream);
                }
                _json_fencode_newline(stream, pretty, depth + 1);
                _json_fencode(stream, jsonarr_getitem(arr, i), pretty, depth + 1);
            }
            _json_fencode_newline(stream, pretty, depth);
            fputc(']', stream);
            break;
        case JSON_OBJECT:
            JsonObject *obj = item->value.as_obj;
            JsonObjectIterator *iter = jsonobj_iter(obj);

            fputc('{', stream);
            while (jsonobj_next(iter)) {
                if (iter->index) {
                    fputc(',', stream);
                }
                _json_fencode_newline(stream, pretty, depth + 1);
                _json_fencode_string(stream, iter->key);
                fputc(':', stream);
                if (pretty) {
                        fputc(' ', stream);
                }
                _json_fencode(stream, iter->value, pretty, depth + 1);
            }
            _json_fencode_newline(stream, pretty, depth);
            fputc('}', stream);
            break;
    }
}


static JsonValue _json_decode(Lexer *lexer, bool *error) {
    Parser *parser = parser_construct(lexer);
    ASTNode *root = parser_parse(parser);
    JsonValue jsval;
    *error = true;

    if (!root) {
        parser_destruct(parser);
        return jsval;
    }

    *error = false;
    jsval = _json_visit(root);
    // On successful parsing, EOF token still remains.
    token_destruct(parser->token);
    parser_destruct(parser);
    ast_destruct(root);
    return jsval;
}


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


/** Decode JSON string and return as a pointer to JsonValue.
 *
 * Set *error* value to true when parsing failed.
 * *text* is not destroyed after decoding.
 */
JsonValue json_sdecode(char *text, bool *error) {
    bool _error;
    Lexer *lexer = lexer_construct(text);
    JsonValue jsval = _json_decode(lexer, &_error);
    *error = _error;
    return jsval;
}

JsonValue *json_fdecode(FILE *json_r) {}


/** Output JsonValue object to file, with optional formatting. */
void json_fencode(FILE *stream, JsonValue *item, bool pretty) {
    _json_fencode(stream, item, pretty, 0);
}
