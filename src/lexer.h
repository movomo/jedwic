#ifndef __JSON_LEXER_H__
#define __JSON_LEXER_H__
#include "token.h"


typedef struct Lexer {
    char *text;
    size_t pos;
    size_t line;
    size_t line_start;
    char chr;
} Lexer;


/** Construct a lexer with given text, keeping copy of the text.
 *
 * Return NULL when memory is low.
 */
Lexer *lexer_construct(char *text);

/** Destruct lexer and also its text. */
void lexer_destruct(Lexer *lexer);

/** Return next token. Token may be EOF or NULL. */
Token *lexer_next(Lexer *lexer);


#endif
