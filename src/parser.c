#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "ast.h"
#include "lexer.h"
#include "parser.h"
#include "token.h"


static ASTNode *_parser_error(Lexer *lexer, TokenKind expected, TokenKind got) {
    fprintf(stderr, "Parser: error:\n");
    return NULL;
}

static ASTNode *_parser_error_memory() {
    fprintf(stderr, "Parser: insufficient memory\n");
    return NULL;
}

static ASTNode *_parser_error_unknown(Token *token) {
    fprintf(stderr, "Parser: error: unrecognized token: %s\n", token->value);
    return NULL;
}


static bool _parser_eat(Parser *parser, TokenKind expected) {
    assert(parser->token != NULL);
    if (parser->token->kind == expected) {
        token_destruct(parser->token);
        parser->token = lexer_next(parser->lexer);
        return true;
    } else {
        _parser_error(parser->lexer, expected, parser->token->kind);
        token_destruct(parser->token);
        return false;
    }
}

static ASTNode *_parser_null(Parser *parser) {
    ASTNode *node = ast_construct_nullnode(parser->token);
    if (!node) {
        return _parser_error_memory();
    }
    if (!_parser_eat(parser, TOKEN_NULL)) {
        return NULL;
    }
    return node;
}

static ASTNode *_parser_bool(Parser *parser) {
    ASTNode *node = ast_construct_boolnode(parser->token);
    if (!node) {
        return _parser_error_memory();
    }
    if (!_parser_eat(parser, TOKEN_BOOL)) {
        return NULL;
    }
    return node;
}

static ASTNode *_parser_number(Parser *parser) {
    ASTNode *node = ast_construct_numbernode(parser->token);
    if (!node) {
        return _parser_error_memory();
    }
    if (!_parser_eat(parser, TOKEN_NUMBER)) {
        return NULL;
    }
    return node;
}

static ASTNode *_parser_string(Parser *parser) {
    ASTNode *node = ast_construct_stringnode(parser->token);
    if (!node) {
        return _parser_error_memory();
    }
    if (!_parser_eat(parser, TOKEN_STRING)) {
        return NULL;
    }
    return node;
}

static ASTNode *_parser_value(Parser *parser) {
    ASTNode *parent;
    ASTNode *child;

    if (!parser->token) {
        return NULL;
    }
    parent = ast_construct_valuenode(parser->token);
    switch(parser->token->kind) {
        case TOKEN_STRING:
            child = _parser_string(parser);
            if (!ast_append(parent, child)) {
                return _parser_error_memory();
            }
            return parent;
        case TOKEN_NUMBER:
            child = _parser_number(parser);
            if (!ast_append(parent, child)) {
                return _parser_error_memory();
            }
            return parent;
        case TOKEN_BOOL:
            child = _parser_bool(parser);
            if (!ast_append(parent, child)) {
                return _parser_error_memory();
            }
            return parent;
        case TOKEN_NULL:
            child = _parser_null(parser);
            if (!ast_append(parent, child)) {
                return _parser_error_memory();
            }
            return parent;
        default:
            return _parser_error_unknown(parser->token);
    }
}

static ASTNode *_parser_element(Parser *parser) {
    return _parser_value(parser);
}

static ASTNode *_parser_json(Parser *parser) {
    return _parser_element(parser);
}


/** Construct a parser and initialize with the first token. */
Parser *parser_construct(Lexer *lexer) {
    Parser *parser = malloc(sizeof (Parser));
    if (!parser) {
        return NULL;
    }
    parser->lexer = lexer;
    parser->token = lexer_next(lexer);
    return parser;
}

/** Destruct parser, but not the lexer. */
void parser_destruct(Parser *parser) {
    free(parser);
}

ASTNode *parser_parse(Parser *parser) {
    return _parser_json(parser);
}
