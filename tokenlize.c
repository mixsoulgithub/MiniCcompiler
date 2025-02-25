#include "mini.h"

//Tokenlize the input string p and return the first token.
Token* Tokenlize(char *p){
    Token head = {};
    Token *cur = &head;
    while (*p) {
        if (isspace(*p)) {
            p++;
            continue;
        }
        if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')'|| *p == ';') {
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
        if('a'<=*p&&*p<='z'){
            cur = cur->next = calloc(1, sizeof(Token));
            cur->kind = TK_ID;
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