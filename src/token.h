#ifndef __JSON_TOKEN_H__
#define __JSON_TOKEN_H__


/**
 * json:        element
 * element:     ws value ws
 * ws:          "" | '\x20' ws | '\x0a' ws | '\x0d' ws | '\x09' ws
 * value:       object | array | string | number | bool | "null"
 * object:      '{' ws '}' | '{' members '}'
 * members:     member | member "," members
 * member:      ws string ws ":" element
 * array:       '[' ws ']' | '[' elements ']'
 * elements:    element | element "," elements
 * string:      '"' characters '"'
 * characters:  "" | character characters
 * character:   '\x20' . '\x10ffff' - '"' - '\' | '\' escape
 * escape:      '"' | '\' | '/' | 'b' | 'f' | 'n' | 'r' | 't' | 'u' hex hex hex hex
 * hex:         digit | 'A' . 'F' | 'a' . 'f'
 * number:      integer fraction exponent
 * integer:     digit | onenine digits | '-' digit | '-' onenine digits
 * digits:      digit | digit digits
 * digit:       '0' | onenine
 * onenine:     '1' . '9'
 * fraction:    "" | '.' digits
 * exponent:    "" | 'E' sign digits | 'e' sign digits
 * sign:        "" | '+' | '-'
 * bool:        "true" | "false"
 */


typedef enum TokenKind {
    TOKEN_EOF,
    // TOKEN_WS,
    TOKEN_NULL,
    TOKEN_BOOL,
    // To remove?
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_EXPONENT_NOTATION,
    TOKEN_FRACTION_DOT,
    TOKEN_DIGIT,
    TOKEN_HEX,
    TOKEN_ESCAPE,
    TOKEN_CHARACTER,
    TOKEN_DOUBLEQUOTE,
    //
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_COLON,
    TOKEN_COMMA,
    TOKEN_OPENING_SQUARE_BRACKET,
    TOKEN_CLOSING_SQUARE_BRACKET,
    TOKEN_OPENING_CURLY_BRACKET,
    TOKEN_CLOSING_CURLY_BRACKET
} TokenKind;

typedef struct Token {
    TokenKind kind;
    char *value;
} Token;


/** Construct a token and keep a copy of value in it. *end* is exclusive.
 *
 * Returns NULL when memory is low.
 */
Token *token_construct(TokenKind kind, char *text, size_t start, size_t end);

/** Destruct token and also destruct its value. */
void token_destruct(Token *token);

/** Print token for debugging. */
void token_print(Token *token);


#endif
