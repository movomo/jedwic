#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "token.h"


/** Construct a token and keep a copy of value in it. *end* is exclusive.
 *
 * Returns NULL when memory is low.
 */
Token *token_construct(TokenKind kind, char *text, size_t start, size_t end) {
    Token *token = malloc(sizeof (Token));
    if (!token) {
        return NULL;
    }
    token->value = calloc(end - start + 1, sizeof (char));
    if (!token->value) {
        free(token);
        return NULL;
    }
    if (!strncpy(token->value, &text[start], end - start)) {
        free(token);
        return NULL;
    }
    token->kind = kind;
    return token;
}

/** Destruct token and also destruct its value. */
void token_destruct(Token *token) {
    free(token->value);
    free(token);
}

/** Print token for debugging. */
void token_print(Token *token) {
    printf("Token { kind: ");
    switch (token->kind) {
        case TOKEN_EOF:
            printf("EOF");
            break;
        case TOKEN_NULL:
            printf("NULL");
            break;
        case TOKEN_BOOL:
            printf("BOOL");
            break;
        case TOKEN_NUMBER:
            printf("NUMBER");
            break;
        case TOKEN_STRING:
            printf("STRING");
            break;
        case TOKEN_COLON:
            printf("COLON");
            break;
        case TOKEN_COMMA:
            printf("COMMA");
            break;
        case TOKEN_OPENING_SQUARE_BRACKET:
            printf("OPENING_SQUARE_BRACKET");
            break;
        case TOKEN_CLOSING_SQUARE_BRACKET:
            printf("CLOSING_SQUARE_BRACKET");
            break;
        case TOKEN_OPENING_CURLY_BRACKET:
            printf("OPENING_CURLY_BRACKET");
            break;
        case TOKEN_CLOSING_CURLY_BRACKET:
            printf("CLOSING_CURLY_BRACKET");
            break;
        default:
            printf("%d", token->kind);
            break;
    }
    printf(", value: %s }\n", token->value);
}
