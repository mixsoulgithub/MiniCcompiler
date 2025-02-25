#ifndef MINI_H
#define MINI_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef enum {
    TK_PUNCT, // Punctuators
    TK_ID,    //identifiers
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

typedef struct ASTnode ASTnode;
struct ASTnode {
    ASTnode *next;
    NodeKind kind;
    int val;
    char name;
    ASTnode *left;
    ASTnode *right;
};

Token* Tokenlize(char *p);
ASTnode* ASTgen(Token *tok);
void codeGen(ASTnode *node);

#endif