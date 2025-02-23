#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef enum {
    TK_PUNCT, // Punctuators
    TK_NUM,   // Numeric literals
    TK_EOF,   // End-of-file markers
  } TokenKind;

typedef struct Token Token;
struct Token {
    TokenKind kind;
    Token *next;
    int val;//if kind is TK_NUM, val is the value of the number.
    char *loc;//Token location in the input string.
    int len;
};

//Tokenlize the input string p and return the first token.
static Token* Tokenlize(char *p){
    Token head = {};
    Token *cur = &head;
    while (*p) {
        if (isspace(*p)) {
            p++;
            continue;
        }
        if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')') {
            cur = cur->next = calloc(1, sizeof(Token));
            cur->kind = TK_PUNCT;
            cur->loc = p;
            cur->len = 1;
            p++;
            continue;
        }
        if (isdigit(*p)) {//only one digit.
            cur = cur->next = calloc(1, sizeof(Token));
            cur->kind = TK_NUM;
            cur->loc = p;
            cur->val = strtol(p, &p, 10);
            continue;
        }
        fprintf(stderr, "unexpected character: '%c'\n", *p);
        exit(1);
    }
    cur = cur->next = calloc(1, sizeof(Token));
    cur->kind = TK_EOF;
    cur->loc = p;
    return head.next;
}

typedef struct ASTnode ASTnode;
struct ASTnode {
    Token* tok;
    ASTnode *left;
    ASTnode *right;
};

static ASTnode* new_node(Token* tok, ASTnode *left, ASTnode *right){
    ASTnode *node = calloc(1, sizeof(ASTnode));
    node->tok = tok;
    // if(tok->kind == TK_NUM){
    //     printf("new node, val: %d\n", tok->val);
    // }else{
    //     printf("new node, loc: %s\n", tok->loc);
    // }
    node->left = left;
    node->right = right;
    return node;
}


//expr = mul ("+" mul | "-" mul)*
static ASTnode* expr(Token **tok_addr);
static ASTnode* mul(Token **tok_addr);
static ASTnode* primary(Token **tok_addr);
static ASTnode* unary(Token **tok_addr);
//tok is the first token of the expression.

static ASTnode* expr(Token **tok_addr){
    // printf("expr invoked\n");
    ASTnode *node = mul(tok_addr);
    Token *tok= *tok_addr;
    while(tok->kind == TK_PUNCT && (*tok->loc == '+' || *tok->loc == '-')){
        Token *op = tok;
        tok = tok->next;
        *tok_addr=tok;
        // printf("tok accessed\n");
        ASTnode *rhs = mul(tok_addr);
        tok= *tok_addr;
        node = new_node(op, node, rhs);//in this layer, node becomes left, so left-associative is guaranteed.
    }
    return node;
}

//mul = unary ("*" unary | "/" unary)*
static ASTnode* mul(Token **tok_addr){
    // printf("mul invoked\n");
    ASTnode *node = unary(tok_addr);
    Token *tok= *tok_addr;
    while(tok->kind == TK_PUNCT && (*tok->loc == '*' || *tok->loc == '/')){
        Token *op = tok;
        tok = tok->next;
        *tok_addr=tok;
        ASTnode *rhs = unary(tok_addr);
        tok= *tok_addr;
        node = new_node(op, node, rhs);
    }
    return node;
}

//unary = ("+"|"-") unary
//      | primary
static ASTnode* unary(Token **tok_addr){
    Token* tok = *tok_addr;
    if(*tok->loc=='+'){
        *tok_addr=tok->next;
        ASTnode *node = unary(tok_addr);
        tok=*tok_addr;
        return node;
    }
    if(*tok->loc=='-'){
        *tok_addr=tok->next;
        ASTnode *node =new_node(tok, NULL, unary(tok_addr));
        tok=*tok_addr;
        return node;
    }
    return primary(tok_addr);
}


//primary = num | "(" expr ")"  
static ASTnode* primary(Token **tok_addr){

    Token *tok = *tok_addr;
    if(tok->kind == TK_NUM){
        ASTnode *node = new_node(tok, NULL, NULL);
        tok = tok->next;
        *tok_addr = tok;
        return node;
    }
    if(tok->kind == TK_PUNCT && *tok->loc == '('){
        tok = tok->next;
        *tok_addr = tok;
        ASTnode *node = expr(tok_addr);
        tok = *tok_addr;
        if(!(tok->kind == TK_PUNCT && *tok->loc == ')')){
            fprintf(stderr, "missing ')'\n");
            exit(1);
        }else{
            tok = tok->next;
            *tok_addr = tok;
        }
        return node;
    }
    fprintf(stderr, "unexpected token %s\n", tok->loc);
    exit(1);
}

static ASTnode *ASTgen(Token *tok){
    return expr(&tok);
}

static void codeGen(ASTnode *node){
    if(node->tok->kind == TK_NUM && node->left == NULL && node->right == NULL){
        printf("  mov $%d, %%rax\n", node->tok->val);
        return;
    }
    if(node->tok->kind == TK_PUNCT && *node->tok->loc == '-'&& node->left == NULL){
        codeGen(node->right);
        printf("  neg %%rax\n");
        return;
    }
    if(node->tok->kind == TK_PUNCT && *node->tok->loc == '+'&& node->left == NULL){
        codeGen(node->right);
        return;
    }
    codeGen(node->right);
    printf("  push %%rax\n");
    codeGen(node->left);
    printf("  pop %%rdi\n");
    if(*node->tok->loc == '+'){
        printf("  add %%rdi, %%rax\n");
    }else if(*node->tok->loc == '-'){
        printf("  sub %%rdi, %%rax\n");
    }else if(*node->tok->loc == '*'){
        printf("  imul %%rdi, %%rax\n");
    }else if(*node->tok->loc == '/'){
        printf("  cqo\n");
        printf("  idiv %%rdi\n");
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr,"Usage: %s <source code string>\n", argv[0]);
        return 1;
    }
    Token *tok = Tokenlize(argv[1]);
    if (tok->kind == TK_EOF) {
        fprintf(stderr, "no input\n");
        return 1;
    }
    // while(tok->kind != TK_EOF){
    //     if(tok->kind == TK_PUNCT){
    //         printf("kind: TK_PUNCT, loc: %s\n", tok->loc);
    //     }else if(tok->kind == TK_NUM){
    //         printf("kind: TK_NUM, val: %d\n", tok->val);
    //     }
    //     tok = tok->next;
    // }
    printf("  .globl main\n");
    printf("main:\n");
    ASTnode *node = ASTgen(tok);
    codeGen(node);
    printf("  ret\n");
    return 0;
}