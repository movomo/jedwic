#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "json.h"
#include "jsonarr.h"
#include "jsonobj.h"
#include "token.h"
#include "ast.h"
#include "lexer.h"
#include "parser.h"


#define NSAMPLES        8


static char *sample[NSAMPLES] = {
    "she", "sells", "sea", "shells", "by", "the", "sea", "shore"
};


int test_arr() {
    JsonArray *array = jsonarr_construct(-1);
    JsonValue jsval;
    jsval.type = JSON_STRING;

    // append/get/pop
    for (size_t i = 0; i < NSAMPLES; i++) {
        jsval.value.as_str = sample[i];
        jsonarr_append(array, &jsval);
    }
    for (size_t i = 0; i < array->len; i++) {
        jsval = *jsonarr_getitem(array, i);
        assert(strcmp(sample[i], jsval.value.as_str) == 0);
    }
    for (size_t i = array->len; i-- > 0; ) {
        jsval = *jsonarr_pop(array);
        assert(strcmp(sample[i], jsval.value.as_str) == 0);
    }

    // insert/set/del/index/clear
    for (size_t i = NSAMPLES; i-- > 0; ) {
        jsval.value.as_str = sample[i];
        jsonarr_insert(array, 0, &jsval);
    }
    jsonarr_setitem(array, NSAMPLES - 1, &jsval);
    jsval.value.as_str = sample[NSAMPLES - 1];
    // jsval changed from "she" to "shore"; arr[7] should be "she";
    assert(!jsonval_equal(jsonarr_getitem(array, NSAMPLES - 1), &jsval));
    jsonarr_delitem(array, NSAMPLES - 1);
    jsonarr_insert(array, NSAMPLES - 1, &jsval);
    for (size_t i = 0; i < array->len; i++) {
        jsval = *jsonarr_getitem(array, i);
        assert(strcmp(sample[i], jsval.value.as_str) == 0);
    }
    assert(jsonarr_index(array, &jsval, -1) == NSAMPLES - 1);
    jsonarr_clear(array);

    // slice/fit/iter/equality
    for (size_t i = 0; i < NSAMPLES; i++) {
        jsval.value.as_str = sample[i];
        jsonarr_append(array, &jsval);
    }

    JsonArray *copy = jsonarr_slice(array, 0, array->len);
    JsonArrayIterator *iter = jsonarr_iter(array);
    JsonValue v1 = { JSON_ARRAY, .value.as_arr = array };
    JsonValue v2 = { JSON_ARRAY, .value.as_arr = copy };

    // iteration and element-wise equality
    jsonarr_fit(copy);
    assert(copy->len == copy->_cap);
    while (jsonarr_next(iter)) {
        // printf("%s vs %s\n", iter->value->value.as_str, jsonarr_getitem(copy, iter->index)->value.as_str);
        assert(jsonval_equal(iter->value, jsonarr_getitem(copy, iter->index)));
    }
    // jsonval_equal functionality
    assert(jsonval_equal(&v1, &v2));

    jsonarr_destruct(array);
    jsonarr_destruct(copy);
    return 1;
}

int test_obj() {
    size_t i;
    JsonObject *obj = jsonobj_construct(json_default_hasher, -1);
    JsonObject *obj2 = jsonobj_construct(json_default_hasher, -1);
    JsonObjectIterator *iter;
    JsonValue jsval = { JSON_NUMBER, .value.as_num = 1 };
    JsonValue jsval2 = { JSON_OBJECT };

    // set/grow
    for (i = 0; i < NSAMPLES; i++) {
        jsval.value.as_num = strlen(sample[i]);
        jsonobj_setitem(obj, sample[i], &jsval);
    }
    // contains/get
    for (size_t i = 0; i < NSAMPLES; i++) {
        assert(jsonobj_contains(obj, sample[i]));
        assert(jsonobj_getitem(obj, sample[i])->value.as_num == strlen(sample[i]));
    }

    // iter/next: If successfully iterate through the whole obj, it's set.
    iter = jsonobj_iter(obj);
    i = 0;
    while (jsonobj_next(iter)) {
        i++;
    }
    assert(i == obj->len);

    // del/shrink
    for (size_t i = 0; i < NSAMPLES; i++) {
        if (jsonobj_contains(obj, sample[i])) {
            jsonobj_delitem(obj, sample[i]);
        }
    }

    // eq/clear/destruct
    for (i = 0; i < NSAMPLES; i++) {
        jsval.value.as_num = strlen(sample[i]);
        jsonobj_setitem(obj, sample[i], &jsval);
        jsonobj_setitem(obj2, sample[i], &jsval);
    }
    jsval.type = JSON_OBJECT;
    jsval.value.as_obj = obj;
    jsval2.value.as_obj = obj2;
    assert(jsonval_equal(&jsval, &jsval2));

    jsonobj_destruct(obj);
    jsonobj_destruct(obj2);
    return 1;
}

void print_tokens(Lexer *lexer) {
    Token *token;
    while ((token = lexer_next(lexer))) {
        token_print(token);
        if (token->kind == TOKEN_EOF)
            break;
    }
}

int test_lexer() {
    Lexer *lexer;
    Token *token;

    lexer = lexer_construct("\
        [\
            true,\
            false,\
            null,\n\
            {\
                \"exp\": 1.2e+10,\
                \"int\": -10,\
                \"float\": 3.14\
            },\
            \"x\\udead\\ubeef\"\
        ]\
    ");
    // print_tokens(lexer);
    while (1) {
        token = lexer_next(lexer);
        assert(token != NULL);
        if (token->kind == TOKEN_EOF) {
            break;
        }
    }
    lexer_destruct(lexer);

    return 1;
}

void gen_ast(char *code) {
    Lexer *lexer;
    Parser *parser;
    ASTNode *node;

    lexer = lexer_construct(code);
    parser = parser_construct(lexer);
    node = parser_parse(parser);
    ast_print_tree(node);
    lexer_destruct(lexer);
    parser_destruct(parser);
    ast_destruct(node);
}

int test_parser() {
    // Lexer *lexer;
    // Parser *parser;
    // ASTNode *node;

    gen_ast("null");

    return 1;
}

int main() {
    if (test_arr()) {
        printf("JsonArray tests passed.\n");
    }

    if (test_obj()) {
        printf("JsonObject tests passed.\n");
    }

    if (test_lexer()) {
        printf("Lexer tests passed.\n");
    }

    if (test_parser()) {
        printf("Parser tests passed.\n");
    }

    return 0;
}
