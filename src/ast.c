#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "token.h"


#define AST_INITIAL_CAPACITY        1
#define AST_GROW_FACTOR             2
#define AST_INDENT                  4


/** Returned string must be freed! */
static char *_ast_get_node_name(ASTKind kind) {
    char *name = malloc(8 * sizeof (char));
    switch (kind) {
        case AST_NULL:
            strcpy(name, "Null");
            break;
        case AST_BOOL:
            strcpy(name, "Bool");
            break;
        case AST_NUMBER:
            strcpy(name, "Number");
            break;
        case AST_STRING:
            strcpy(name, "String");
            break;
        case AST_ARRAY:
            strcpy(name, "Array");
            break;
        case AST_OBJECT:
            strcpy(name, "Object");
            break;
        case AST_KEY:
            strcpy(name, "Key");
            break;
        default:
            assert(0);
    }
    return name;
}

static void _ast_print_node(ASTNode *node, int depth) {
    depth *= AST_INDENT;
    char *name = _ast_get_node_name(node->kind);
    for (int i = 0; i < depth; i++) {
        if ((i + 2) % AST_INDENT == 0) {
            if (i == depth - 2) {
                printf("L");
            } else {
                printf("|");
            }
        } else {
            printf(" ");
        }
    }
    printf(
        "%sNode { .value = %s, .len = %llu }\n",
        name,
        node->value,
        node->len
    );
    free(name);
}

static void *_ast_print_tree(ASTNode *root, int depth) {
    size_t i;
    _ast_print_node(root, depth);
    for (i = 0; i < root->len; i++) {
        _ast_print_tree(root->children[i], depth + 1);
    }
}

/** Construct a generic node and keep a copy of *value*. */
static ASTNode *_ast_construct(char *value, size_t capacity) {
    ASTNode *node = calloc(1, sizeof (ASTNode));
    size_t val_len = strlen(value);

    if (!node) {
        return NULL;
    }
    if (capacity == SIZE_MAX) {
        capacity = 1;
    }
    node->children = malloc(capacity * sizeof (ASTNode *));
    if (!node->children) {
        return NULL;
    }
    node->len = 0;
    node->cap = capacity;
    node->value = malloc((val_len + 1) * sizeof (char));
    if (!node->value) {
        return NULL;
    }
    if (!memcpy(node->value, value, val_len + 1)) {
        return NULL;
    }
    return node;
}

static bool _ast_grow(ASTNode *node) {
    ASTNode **new_children;
    size_t new_cap;

    if (node->len < node->cap) {
        return true;
    } else {
        new_cap = node->cap * AST_GROW_FACTOR;
        new_children = malloc(new_cap * sizeof (ASTNode *));
        if (!new_children) {
            return false;
        } else {
            if (!memcpy(new_children,
                        node->children,
                        node->len * sizeof (ASTNode *))) {
                free(new_children);
                return false;
            }
        }
    }
    free(node->children);
    node->children = new_children;
    node->cap = new_cap;
    return true;
}


/** Construct a null node and return its pointer or NULL. */
ASTNode *ast_construct_nullnode(Token *token) {
    ASTNode *node = _ast_construct(token->value, 0);

    if (!node) {
        return NULL;
    }
    node->kind = AST_NULL;
    return node;
}

/** Construct a true/false node and return its pointer or NULL. */
ASTNode *ast_construct_boolnode(Token *token) {
    ASTNode *node = _ast_construct(token->value, 0);

    if (!node) {
        return NULL;
    }
    node->kind = AST_BOOL;
    return node;
}

/** Construct a number node and return its pointer or NULL. */
ASTNode *ast_construct_numbernode(Token *token) {
    ASTNode *node = _ast_construct(token->value, 0);

    if (!node) {
        return NULL;
    }
    node->kind = AST_NUMBER;
    return node;
}

/** Construct a string node and return its pointer or NULL. */
ASTNode *ast_construct_stringnode(Token *token) {
    ASTNode *node = _ast_construct(token->value, 0);

    if (!node) {
        return NULL;
    }
    node->kind = AST_STRING;
    return node;
}

/** Construct an array node and return its pointer or NULL. */
ASTNode *ast_construct_arraynode(Token *token) {
    ASTNode *node = _ast_construct(token->value, 4);

    if (!node) {
        return NULL;
    }
    node->kind = AST_ARRAY;
    return node;
}

/** Construct a key node and return its pointer or NULL. */
ASTNode *ast_construct_keynode(Token *token) {
    ASTNode *node = _ast_construct(token->value, 1);

    if (!node) {
        return NULL;
    }
    node->kind = AST_KEY;
    return node;
}

/** Construct an object node and return its pointer or NULL. */
ASTNode *ast_construct_objectnode(Token *token) {
    ASTNode *node = _ast_construct(token->value, 4);

    if (!node) {
        return NULL;
    }
    node->kind = AST_OBJECT;
    return node;
}

/** Recursively destruct entrie AST starting from *root*. */
void *ast_destruct(ASTNode *root) {
    size_t i;
    if (root) {
        for (i = 0; i < root->len; i++) {
            ast_destruct(root->children[i]);
        }
        ast_destruct_node(root);
    }
}

/** Destruct a single AST node as well as textual values. */
void *ast_destruct_node(ASTNode *node) {
    if (node) {
        free(node->value);
        free(node->children);
        free(node);
    }
}

/** Append a child node to the parent node and report success as bool. */
bool ast_append(ASTNode *parent, ASTNode *child) {
    if (!_ast_grow(parent)) {
        return false;
    }
    parent->children[parent->len++] = child;
    return true;
}

/** Print a single AST node. */
void *ast_print_node(ASTNode *node) {
    _ast_print_node(node, 0);
}

/** Print entire AST. */
void *ast_print_tree(ASTNode *root) {
    _ast_print_tree(root, 0);
}
