#ifndef MINI_H
#define MINI_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef enum {
    TK_PUNCT, // Punctuators
    TK_ID,    //identifiers
    TK_KEYWORD, //keywords
    TK_NUM,   // Numeric literals
    TK_EOF,   // End-of-file markers
  } TokenKind;

typedef enum {
    ND_ADD, // +
    ND_SUB, // -
    ND_MUL, // *
    ND_DIV, // /
    ND_NEG, // unary -
    ND_EQ,  // ==
    ND_NE,  // !=
    ND_LT,  // <
    ND_LE,  // <=
    ND_SEMI, // ;
    ND_EMPTY, //empty sentence
    ND_ASSIGN, // =
    ND_RETURN, // return
    ND_BLOCK, // block
    ND_IF, // if
    ND_FOR, // for
    ND_VAR, // Variable
    ND_NUM, // Integer
  } NodeKind;

typedef struct Token Token;
struct Token {
    TokenKind kind;
    Token *next;
    int val;//if kind is TK_NUM, val is the value of the number.
    char *loc;//Token location in the input string.
    int len;
};

typedef struct LocalVar LocalVar;
struct LocalVar {
    char *name;
    LocalVar *next;
    int offset;
};

typedef struct ASTnode ASTnode;
struct ASTnode {
    ASTnode *next;
    NodeKind kind;
    int val;
    LocalVar *var;//if kind is ND_VAR, var is the variable.
    ASTnode *body;//if kind is ND_BLOCK, body is the block.

    ASTnode *cond, *then, *els;
    ASTnode *init, *inc;

    ASTnode *left;
    ASTnode *right;
};

extern LocalVar *locals;
extern int equal(Token *tok, char *op);

Token* Tokenlize(char *p);
ASTnode* ASTgen(Token *tok);
void codeGen(ASTnode *node);

#endif