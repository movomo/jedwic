#ifndef __JSON_PARSER_H__
#define __JSON_PARSER_H__

#include "lexer.h"
#include "token.h"


typedef struct Parser {
    Lexer *lexer;
    Token *token;
} Parser;


/** Construct a parser and initialize with the first token. */
Parser *parser_construct(Lexer *lexer);

/** Destruct parser, but not the lexer.
 *
 * Last token needs to be destroyed manually if parsing didn't error out.
 */
void parser_destruct(Parser *parser);

ASTNode *parser_parse(Parser *parser);


#endif
