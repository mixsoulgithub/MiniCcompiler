#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef enum {
    TK_PUNCT, // Punctuators
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
        if(*p== '='||*p == '<'||*p == '>'||*p == '!'){
            if(*(p+1) == '='){
                cur = cur->next = calloc(1, sizeof(Token));
                cur->kind = TK_PUNCT;
                cur->loc = p;
                cur->len = 2;
                p+=2;
            }else{
                cur = cur->next = calloc(1, sizeof(Token));
                cur->kind = TK_PUNCT;
                cur->loc = p;
                cur->len = 1;
                p++;
            }
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

static int equal(Token *tok, char *node_tok){
    return strlen(node_tok) == tok->len && !memcmp(tok->loc, node_tok, tok->len);
}

typedef struct ASTnode ASTnode;
struct ASTnode {
    NodeKind kind;
    int val;
    ASTnode *left;
    ASTnode *right;
};

static ASTnode* new_node(NodeKind kind, ASTnode *left, ASTnode *right){
    ASTnode *node = calloc(1, sizeof(ASTnode));
    node->kind = kind;
    // fprintf(stderr,"new a node. kind is : %d\n", node->kind);
    node->left = left;
    node->right = right;
    return node;
}

static ASTnode* new_numnode(NodeKind kind, int num, ASTnode *left, ASTnode *right){
    ASTnode *node = new_node(kind, left, right);
    node->val=num;
    return node;
}//many number nodes are created, it's a waste to use "if" to distinguish them in new_node function.

static ASTnode* expr(Token **tok_addr);
static ASTnode* equaility(Token **tok_addr);
static ASTnode* relational(Token **tok_addr);
static ASTnode* add(Token **tok_addr);
static ASTnode* mul(Token **tok_addr);
static ASTnode* primary(Token **tok_addr);
static ASTnode* unary(Token **tok_addr);
//tok is the first token of the expression.

//expr = equiality
static ASTnode* expr(Token **tok_addr){
    return equaility(tok_addr);
}

//equiality = relational ("==" relational | "!=" relational)*
static ASTnode* equaility(Token **tok_addr){
    ASTnode *node = relational(tok_addr);
    Token *tok= *tok_addr;
    while(tok->kind == TK_PUNCT && (equal(tok, "==") || equal(tok, "!="))){
        Token *node_tok = tok;
        tok = tok->next;

        *tok_addr=tok;
        ASTnode *rhs = relational(tok_addr);
        tok= *tok_addr;

        if(equal(node_tok, "==")){
            node = new_node(ND_EQ, node, rhs);
        }else{
            node = new_node(ND_NE, node, rhs);
        }//left-associative too.
    }
    return node;
}

//relational = add ("<" add | "<=" add | ">" add | ">=" add)*
static ASTnode* relational(Token **tok_addr){
    ASTnode *node = add(tok_addr);
    Token *tok= *tok_addr;
    while(tok->kind == TK_PUNCT && (equal(tok, "<") || equal(tok, "<=") || equal(tok, ">") || equal(tok, ">="))){
        Token *node_tok = tok;
        tok = tok->next;

        *tok_addr=tok;
        ASTnode *rhs = add(tok_addr);
        tok= *tok_addr;

        if(equal(node_tok, "<")){
            node = new_node(ND_LT, node, rhs);
        }else if(equal(node_tok, "<=")){   
            node = new_node(ND_LE, node, rhs);
        }else if (equal(node_tok, ">")){
            node = new_node(ND_LT, rhs, node);//left-assosicative? but 1<2<3 is not allowed in C.
        }else{
            node = new_node(ND_LE, rhs, node);
        }
    }
    return node;
}

//add = mul ("+" mul | "-" mul)*
static ASTnode* add(Token **tok_addr){
    // printf("expr invoked\n");
    ASTnode *node = mul(tok_addr);
    Token *tok= *tok_addr;
    while(tok->kind == TK_PUNCT && (*tok->loc == '+' || *tok->loc == '-')){
        Token *node_tok = tok;
        tok = tok->next;

        *tok_addr=tok;
        ASTnode *rhs = mul(tok_addr);
        tok= *tok_addr;

        if(*node_tok->loc == '+'){
            node = new_node(ND_ADD, node, rhs);
        }else{
            node = new_node(ND_SUB, node, rhs);//in this layer, node becomes left, so left-associative is guaranteed.
        }
    }
    return node;
}

//mul = unary ("*" unary | "/" unary)*
static ASTnode* mul(Token **tok_addr){
    ASTnode *node = unary(tok_addr);
    Token *tok= *tok_addr;

    while(tok->kind == TK_PUNCT && (*tok->loc == '*' || *tok->loc == '/')){
        Token *node_tok = tok;
        tok = tok->next;

        *tok_addr=tok;
        ASTnode *rhs = unary(tok_addr);
        tok= *tok_addr;
        
        if(*node_tok->loc == '/'){
            node = new_node(ND_DIV, node, rhs);
        }else{
            node = new_node(ND_MUL, node, rhs);
        }
    }
    return node;
}

//unary = ("+"|"-") unary
//      | primary
static ASTnode* unary(Token **tok_addr){
    Token* tok = *tok_addr;
    if(*tok->loc=='+'){
        *tok_addr=tok->next;
        ASTnode *node = unary(tok_addr);//unary + not even enter parsing tree.
        tok=*tok_addr;
        return node;
    }
    if(*tok->loc=='-'){
        *tok_addr=tok->next;
        ASTnode *node =new_node(ND_NEG, NULL, unary(tok_addr));
        tok=*tok_addr;
        return node;
    }
    return primary(tok_addr);
}


//primary = num | "(" expr ")"  
static ASTnode* primary(Token **tok_addr){

    Token *tok = *tok_addr;
    if(tok->kind == TK_NUM){
        ASTnode *node = new_numnode(ND_NUM, tok->val, NULL, NULL);
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
    // if(node->tok->kind == TK_NUM && node->left == NULL && node->right == NULL){
    //     printf("  mov $%d, %%rax\n", node->tok->val);
    //     return;
    // }
    // if(node->tok->kind == TK_PUNCT && *node->tok->loc == '-'&& node->left == NULL){
    //     codeGen(node->right);
    //     printf("  neg %%rax\n");
    //     return;
    // }
    // if(node->tok->kind == TK_PUNCT && *node->tok->loc == '+'&& node->left == NULL){
    //     codeGen(node->right);
    //     return;
    // }
    if(node->right!=NULL&&node->left!=NULL){//handle normal codition together in case stack imbalance.
        codeGen(node->right);
        printf("  push %%rax\n");
        codeGen(node->left);
        printf("  pop %%rdi\n");
    }
    switch (node->kind)
    {
    case ND_NUM:
        printf("  mov $%d, %%rax\n", node->val);
        break;
    case ND_ADD:
        printf("  add %%rdi, %%rax\n");
        break;
    case ND_SUB:
        printf("  sub %%rdi, %%rax\n");
        break;
    case ND_MUL:
        printf("  imul %%rdi, %%rax\n");
        break;
    case ND_DIV:
        printf("  cqo\n");
        printf("  idiv %%rdi\n");
        break;
    case ND_NEG:
        codeGen(node->right);
        printf("  neg %%rax\n");
        break;
    case ND_EQ:
    case ND_NE:
    case ND_LT:
    case ND_LE:
        printf("  cmp %%rdi, %%rax\n");
    
        if (node->kind == ND_EQ)
        printf("  sete %%al\n");
        else if (node->kind == ND_NE)
        printf("  setne %%al\n");
        else if (node->kind == ND_LT)
        printf("  setl %%al\n");
        else if (node->kind == ND_LE)
        printf("  setle %%al\n");
    
        printf("  movzb %%al, %%rax\n");
        break;
    default:
        fprintf(stderr,"invalid node kind in codeGen\n");
        return;
    }
    return;
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