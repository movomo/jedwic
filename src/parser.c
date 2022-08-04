#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "ast.h"
#include "lexer.h"
#include "parser.h"
#include "token.h"


static ASTNode *_parser_value(Parser *parser);


static ASTNode *_parser_error(Lexer *lexer, TokenKind expected, TokenKind got) {
    fprintf(stderr, "\nParser: error:\n");
    return NULL;
}

static ASTNode *_parser_error_memory() {
    fprintf(stderr, "\nParser: insufficient memory\n");
    return NULL;
}

static ASTNode *_parser_error_unexpected(Token *token) {
    fprintf(stderr, "\nParser: error: unexpected token: %s\n", token->value);
    return NULL;
}


static bool _parser_eat(Parser *parser, TokenKind expected) {
    if (!parser->token) {
        return false;
    } else if (parser->token->kind == expected) {
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
        ast_destruct_node(node);
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
        ast_destruct_node(node);
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
        ast_destruct_node(node);
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
        ast_destruct_node(node);
        return NULL;
    }
    return node;
}

static ASTNode *_parser_array(Parser *parser) {
    ASTNode *parent = ast_construct_arraynode(parser->token);
    ASTNode *child;
    Token **token = &parser->token;

    if (!parent) {
        return _parser_error_memory();
    }
    if (!_parser_eat(parser, TOKEN_OPENING_SQUARE_BRACKET)) {
        ast_destruct_node(parent);
        return NULL;
    }

    if (*token && (*token)->kind != TOKEN_CLOSING_SQUARE_BRACKET) {
        if (!(child = _parser_value(parser))) {
            ast_destruct(parent);
            return NULL;
        }
        if (!ast_append(parent, child)) {
            ast_destruct(parent);
            return _parser_error_memory();
        }

        while (*token && (*token)->kind == TOKEN_COMMA) {
            if (!_parser_eat(parser, TOKEN_COMMA)) {
                ast_destruct(parent);
                return NULL;
            }

            if (!(child = _parser_value(parser))) {
                ast_destruct(parent);
                return NULL;
            }
            if (!ast_append(parent, child)) {
                ast_destruct(parent);
                return _parser_error_memory();
            }
        }
    }

    if (!_parser_eat(parser, TOKEN_CLOSING_SQUARE_BRACKET)) {
        ast_destruct(parent);
        return NULL;
    }
    return parent;
}

static ASTNode *_parser_key(Parser *parser) {
    ASTNode *parent = ast_construct_keynode(parser->token);
    ASTNode *child;

    if (!parent) {
        return _parser_error_memory();
    }
    if (!_parser_eat(parser, TOKEN_STRING)) {
        ast_destruct_node(parent);
        return NULL;
    }
    
    // colon
    if (!_parser_eat(parser, TOKEN_COLON)) {
        ast_destruct_node(parent);
        ast_destruct_node(child);
        return NULL;
    }

    // value
    if (!(child = _parser_value(parser))) {
        ast_destruct_node(parent);
        ast_destruct_node(child);
        return NULL;
    }
    if (!ast_append(parent, child)) {
        ast_destruct_node(parent);
        ast_destruct_node(child);
        return _parser_error_memory();
    }

    return parent;
}

static ASTNode *_parser_object(Parser *parser) {
    ASTNode *parent = ast_construct_objectnode(parser->token);
    ASTNode *child;
    Token **token = &parser->token;

    if (!parent) {
        return _parser_error_memory();
    }
    if (!_parser_eat(parser, TOKEN_OPENING_CURLY_BRACKET)) {
        return NULL;
    }

    if (*token && (*token)->kind != TOKEN_CLOSING_CURLY_BRACKET) {
        if (!(child = _parser_key(parser))) {
            ast_destruct(parent);
            return NULL;
        }
        if (!ast_append(parent, child)) {
            ast_destruct(parent);
            return _parser_error_memory();
        }

        while (*token && (*token)->kind == TOKEN_COMMA) {
            if (!_parser_eat(parser, TOKEN_COMMA)) {
                ast_destruct(parent);
                return NULL;
            }

            if (!(child = _parser_key(parser))) {
                ast_destruct(parent);
                return NULL;
            }
            if (!ast_append(parent, child)) {
                ast_destruct(parent);
                return _parser_error_memory();
            }
        }
    }

    if (!_parser_eat(parser, TOKEN_CLOSING_CURLY_BRACKET)) {
        ast_destruct(parent);
        return NULL;
    }
    return parent;
}

static ASTNode *_parser_value(Parser *parser) {
    ASTNode *node;

    if (!parser->token) {
        return NULL;
    }
    switch(parser->token->kind) {
        case TOKEN_STRING:
            return _parser_string(parser);
        case TOKEN_BOOL:
            return _parser_bool(parser);
        case TOKEN_NUMBER:
            return _parser_number(parser);
        case TOKEN_OPENING_SQUARE_BRACKET:
            return _parser_array(parser);
        case TOKEN_OPENING_CURLY_BRACKET:
            return _parser_object(parser);
        case TOKEN_NULL:
            return _parser_null(parser);
        default:
            return _parser_error_unexpected(parser->token);
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
