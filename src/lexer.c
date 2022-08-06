#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "token.h"


/** Ignore ' ', '\t', '\n', '\r' */
#define _ISSPACE(c)         \
    ((c) == 0x20 || (c) == 0x09 || (c) == 0x0a || (c) == 0x0d)


enum NumberPhase {
    PHASE_INTEGER,
    PHASE_FRACTION,
    PHASE_EXPONENT
};


Token *_lexer_error(Lexer *lexer, char *msg) {
    char *p = &lexer->text[lexer->line_start];
    size_t len = lexer->pos - lexer->line_start;
    size_t pos;
    size_t pos_oneth;

    fprintf(stderr, "\nLexer: error: ");
    fprintf(
        stderr,
        "%s (line %llu, pos %llu)\n",
        msg,
        lexer->line,
        len
    );

    do {
        fprintf(stderr, "%c", *p++);
    } while (*p != '\n' && *p);
    fprintf(stderr, "\n");
    // Super helpful error indicator!
    for (pos = 0; pos < len; pos++) {
        if (pos % 10) {
            fprintf(stderr, "_");
        } else {
            pos_oneth = pos / 10;
            while (pos_oneth >= 10) {
                pos_oneth /= 10;
            }
            fprintf(stderr, "%llu", pos_oneth);
        }
    }
    fprintf(stderr, "^\n");

    return NULL;
}


void _lexer_advance(Lexer *lexer) {
    if (lexer->chr == '\n') {
        lexer->line++;
        lexer->line_start = lexer->pos + 1;
    }
    lexer->chr = lexer->text[++lexer->pos];
}

void _lexer_skip_ws(Lexer *lexer) {
    char *chr = &lexer->chr;
    while (_ISSPACE(*chr) && *chr != 0) {
        _lexer_advance(lexer);
    }
}

Token *_lexer_const(Lexer *lexer, TokenKind expected, char* ref) {
    char *chr = &lexer->chr;
    size_t start = lexer->pos;
    size_t n = 0;
    while (isalpha(*chr)) {
        _lexer_advance(lexer);
        n++;
    }
    if (strncmp(&(lexer->text[start]), ref, n)) {
        return _lexer_error(lexer, "invalid literal");
    } else {
        return token_construct(expected, lexer->text, start, lexer->pos);
    }
}

/** Look ahead far away to estimate needed buffer size. */
size_t _lexer_string_recon(Lexer *lexer) {
    size_t bufsize = 1;
    bool esc = false;
    char *chr = &lexer->text[lexer->pos];

    while (*chr && (*chr != '"' || esc)) {
        if (esc) {
            esc = false;
            if (*chr == 'u') {
                bufsize++;
            }
        } else {
            esc = (*chr == '\\');
            bufsize++;
        }
        chr++;
    }

    return bufsize;
}

bool _lexer_hex(Lexer *lexer, char *buf, size_t *index) {
    size_t i;
    char *chr = &lexer->chr;
    for (i = 0; i < 4; i++) {
        if (isxdigit(*chr)) {
            buf[(*index)++] = *chr;
            _lexer_advance(lexer);
        } else {
            return false;
        }
    }
    return true;
}

Token *_lexer_string(Lexer *lexer) {
    size_t start = lexer->pos;
    size_t start_local;
    size_t i = 0;
    bool esc = false;
    char *chr = &lexer->chr;
    char *buf = calloc(_lexer_string_recon(lexer), sizeof (char));
    Token *token;

    if (!buf) {
        return NULL;
    }

    while (*chr >= 0x20 && (*chr != '"' || esc)) {
        if (esc) {
            esc = false;
            switch (*chr) {
                case '"':
                case '\\':
                case '/':
                    buf[i++] = *chr;
                    _lexer_advance(lexer);
                    break;
                case 'b':
                    buf[i++] = '\b';
                    _lexer_advance(lexer);
                    break;
                case 'f':
                    buf[i++] = '\f';
                    _lexer_advance(lexer);
                    break;
                case 'n':
                    buf[i++] = '\n';
                    _lexer_advance(lexer);
                    break;
                case 'r':
                    buf[i++] = '\r';
                    _lexer_advance(lexer);
                    break;
                case 't':
                    buf[i++] = '\t';
                    _lexer_advance(lexer);
                    break;
                case 'u':
                    // Need to process unicode sequence here.
                    buf[i++] = '\\';
                    buf[i++] = 'u';
                    _lexer_advance(lexer);
                    if (!_lexer_hex(lexer, buf, &i)) {
                        free(buf);
                        return _lexer_error(lexer, "illegal unicode sequence");
                    }
                    break;
                default:
                    // Unrecognized escape.
                    free(buf);
                    return _lexer_error(lexer, "illegal escape");
            }
        } else {
            if (*chr == '\\') {
                esc = true;
            } else {
                buf[i++] = *chr;
            }
            _lexer_advance(lexer);
        }
    }

    if (!*chr) {
        free(buf);
        return _lexer_error(lexer, "EOF reached while parsing string");
    } else {
        _lexer_advance(lexer);
    }

    token = token_construct(TOKEN_STRING, buf, 0, i);
    free(buf);
    return token;
}

Token *_lexer_number(Lexer *lexer) {
    // A valid json number is also a valid c double.
    // We can simply collect the entire sequence and pass it to strtod.
    size_t start = lexer->pos;
    size_t i = 0;
    char *chr = &lexer->chr;

    bool leading_zero = false;
    enum NumberPhase phase = PHASE_INTEGER;

    // integer: digit | onenine digits | '-' digit | '-' onenine digits
    if (*chr == '-') {
        _lexer_advance(lexer);
    }
    while (isdigit(*chr)) {
        if (i == 0) {
            if (*chr == '0') {
                leading_zero = true;
            }
        } else if (leading_zero) {
            return _lexer_error(lexer, "illegal integer sequence");
        }
        i++;
        _lexer_advance(lexer);
    }
    if (i == 0) {
        // leading sign followed by no digit.
        return _lexer_error(lexer, "sign not followed by digits");
    }

    // fraction: "" | '.' digits
    if (*chr == '.') {
        _lexer_advance(lexer);
        i = 0;
        while (isdigit(*chr)) {
            i++;
            _lexer_advance(lexer);
        }
        if (i == 0) {
            // dot without digit.
            return _lexer_error(lexer, "dot not followed by digits");
        }
    }

    // exponent: "" | 'E' sign digits | 'e' sign digits
    if (*chr == 'e' || *chr == 'E') {
        _lexer_advance(lexer);
        if (*chr == '+' || *chr == '-') {
            _lexer_advance(lexer);
        }
        i = 0;
        while (isdigit(*chr)) {
            i++;
            _lexer_advance(lexer);
        }
        if (i == 0) {
            // exponent without digit.
            return _lexer_error(lexer, "incomplete exponent");
        }
    }

    return token_construct(TOKEN_NUMBER, lexer->text, start, lexer->pos);
}


/** Construct a lexer with given text, keeping copy of the text.
 *
 * Return NULL when memory is low.
 */
Lexer *lexer_construct(char *text) {
    Lexer *lexer = malloc(sizeof (Lexer));
    size_t len;

    if (!lexer) {
        return NULL;
    }
    len = strlen(text);
    lexer->text = malloc((len + 1) * sizeof (char));
    if (!lexer->text) {
        free(lexer);
        return NULL;
    }
    if (!memcpy(lexer->text, text, len + 1)) {
        free(lexer->text);
        free(lexer);
        return NULL;
    }
    lexer->pos = 0;
    lexer->line = 1;
    lexer->line_start = 0;
    lexer->chr = lexer->text[lexer->pos];
    return lexer;
}

/** Destruct lexer and also its text. */
void lexer_destruct(Lexer *lexer) {
    free(lexer->text);
    free(lexer);
}

/** Return next token. Token may be EOF or NULL. */
Token *lexer_next(Lexer *lexer) {
    char *chr = &lexer->chr;
    size_t start;
    size_t end;
    Token *token;

    while (*chr != 0) {
        _lexer_skip_ws(lexer);
        start = lexer->pos;
        end = start + 1;

        switch (*chr) {
            case 'n':
                return _lexer_const(lexer, TOKEN_NULL, "null");
            case 't':
                return _lexer_const(lexer, TOKEN_BOOL, "true");
            case 'f':
                return _lexer_const(lexer, TOKEN_BOOL, "false");
            case '"':
                _lexer_advance(lexer);
                token = _lexer_string(lexer);
                return token;
            case ':':
                token = token_construct(TOKEN_COLON, lexer->text, start, end);
                _lexer_advance(lexer);
                return token;
            case '{':
                token = token_construct(
                    TOKEN_OPENING_CURLY_BRACKET, lexer->text, start, end
                );
                _lexer_advance(lexer);
                return token;
            case '}':
                token = token_construct(
                    TOKEN_CLOSING_CURLY_BRACKET, lexer->text, start, end
                );
                _lexer_advance(lexer);
                return token;
            case '[':
                token = token_construct(
                    TOKEN_OPENING_SQUARE_BRACKET, lexer->text, start, end
                );
                _lexer_advance(lexer);
                return token;
            case ']':
                token = token_construct(
                    TOKEN_CLOSING_SQUARE_BRACKET, lexer->text, start, end
                );
                _lexer_advance(lexer);
                return token;
            case ',':
                token = token_construct(TOKEN_COMMA, lexer->text, start, end);
                _lexer_advance(lexer);
                return token;
            case '-':
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                return _lexer_number(lexer);
            case 0:
                return token_construct(TOKEN_EOF, "EOF", 0, 3);
            default:
                return _lexer_error(lexer, "unrecognized token");
                break;
        }
    }

    return token_construct(TOKEN_EOF, "EOF", 0, 3);
}
