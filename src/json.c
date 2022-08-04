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
        default:
            assert(0);
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


void json_fencode(FILE *stream, JsonValue *item) {
    // size_t i;
    // size_t len;
    
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
            char *chr = item->value.as_str;
            fputc('"', stream);
            while (*chr) {
                switch (*chr) {
                    case '"':
                    case '\\':
                    case '\n':
                    case '\r':
                    case '\t':
                    case '\b':
                    case '\f':
                        fputc('\\', stream);
                        fputc(*chr, stream);
                        break;
                    default:
                        fputc(*chr, stream);
                        break;
                }
                chr++;
            }
            fputc('"', stream);
            break;
        case JSON_ARRAY:
            fprintf(stream, "[");
            break;
        case JSON_OBJECT:
            break;
    }
}
