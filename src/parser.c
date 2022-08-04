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


static bool _parser_eat(Parser *parser, TokenKind expected) {
    assert(parser->token != NULL);
    if (parser->token->kind == expected) {
        token_destruct(parser->token);
        parser->token = lexer_next(parser->lexer);
        return true;
    } else {
        _parser_error(parser->lexer, expected, parser->token->kind);
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
    return _parser_bool(parser);
}
