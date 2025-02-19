#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef enum {
    TK_PUNCT, // Punctuators
    TK_NUM,   // Numeric literals
    TK_EOF,   // End-of-file markers
    TK_LEFTL, // Left parenthesis
    TK_RIGHTL// Right parenthesis
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
        if (*p == '+' || *p == '-') {
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

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr,"Usage: %s <source code string>\n", argv[0]);
        return 1;
    }
    Token *tok = Tokenlize(argv[1]);
    printf("  .globl main\n");
    printf("main:\n");
    while(tok->kind != TK_EOF){//use eof rather than NULL
        if(tok->kind == TK_NUM){
            printf("  mov $%d, %%rax\n", tok->val);//in fact, it will excute one time if the input string is valid.
        }else if(tok->kind == TK_PUNCT){//punct has only one character, easy to handle.
            if(*tok->loc == '+'){
                tok = tok->next;
                printf("  add $%d, %%rax\n", tok->val);
            }else if(*tok->loc == '-'){
                tok = tok->next;
                printf("  sub $%d, %%rax\n", tok->val);
            }
        } 
        tok = tok->next;
    }
    printf("  ret\n");
    return 0;
}