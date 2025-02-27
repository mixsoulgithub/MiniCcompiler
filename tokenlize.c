#include "mini.h"

static int legal_var_name1(char c){
    if('a'<=c&&c<='z'||'A'<=c&&c<='Z'||c=='_'){
        return 1;
    }
    return 0;
}

static int legal_var_name2(char c){
    if('a'<=c&&c<='z'||'A'<=c&&c<='Z'||c=='_'||'0'<=c&&c<='9'){
        return 1;
    }
    return 0;
}

// static int isKeyword(Token *tok){
//     if(tok->kind == TK_ID){
//         if(equal(tok, "if")){
//             return 1;
//         }
//         if(equal(tok, "else")){
//             return 1;
//         }
//         if(equal(tok, "while")){
//             return 1;
//         }
//         if(equal(tok, "for")){
//             return 1;
//         }
//         if(equal(tok, "return")){
//             tok->kind = TK_KEYWORD;
//             return 1;
//         }
//     }
//     return 0;
// }

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
        if(legal_var_name1(*p)){
            char *q = p;
            while(legal_var_name2(*p)){
                p++;
            }
            cur = cur->next = calloc(1, sizeof(Token));
            cur->kind = TK_ID;
            cur->loc = q;
            cur->len = p-q;
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