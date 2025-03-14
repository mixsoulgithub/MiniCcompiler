#ifndef MINI_H
#define MINI_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
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
    ND_ADDR, // &
    ND_DEREF, // *
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
    ND_WHILE,//while
    ND_DEC,//declare
    ND_VAR, // Variable
    ND_NUM, // Integer
    ND_FUNCALL,//function call
    ND_FUNDEF,//global ...,frame.
  } NodeKind;

typedef enum {
  TY_INT,
  TY_POINTER,
  TY_ALL,
} TypeKind;

typedef struct Type Type;
struct Type {
  TypeKind tykind;
  Type *base;
};

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
    Type* type;
    int offset;
};

typedef struct Scope Scope;
struct Scope{
  Scope* before;
  LocalVar* locals;
};

typedef struct Function Function;
struct Function{
    char* name;
    LocalVar* args;
    int argn;
    Scope* scope;
    int fs;//frame size
    int curfs;//current frame size
    Type* type;
    Function* next;
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
    Function* func;

    ASTnode *left;
    ASTnode *right;
    Type *type;
};


extern LocalVar *locals;
extern int equal(Token *tok, char *op);
extern Type* ty_int;
extern Type* point_to(Type *ty);
extern Type* getNodeType(ASTnode* node);
extern int isBaseType(Token *tok);
extern Type* getbasetype(Token*tok);

Token* Tokenlize(char *p);
ASTnode* ASTgen(Token *tok);
void codeGen(ASTnode *node);

#endif