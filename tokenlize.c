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
 
int isKeyword(Token *tok){
    if(tok->kind == TK_ID){
        return equal(tok, "if")     || equal(tok, "else")       || equal(tok, "while")
            || equal(tok, "for")    || equal(tok, "return")     || equal(tok, "int");
    }
    return 0;
}

int isBaseType(Token *tok){
    if(tok->kind==TK_KEYWORD){
        return equal(tok, "int") || equal(tok, "char");
    }
    return 0;
}

Type * getbasetype(Token *tok){
    if(equal(tok, "int")){
        return ty_int;
    }
    fprintf(stderr,"<%s>: undefined type",__func__);
}

Type * matchBasicType(Token* tok){
    if(equal(tok,"int")){
        return ty_int; 
    }
    return NULL;
}

//Tokenlize the input string p and return the first token.
Token* Tokenlize(char *p){
    Token head = {};
    Token *cur = &head;
    while (*p) {
        if (isspace(*p)) {
            p++;
            continue;
        }
        switch (*p)
        {
        case '+':
        case '-':
        case '*':
        case '/':
        case '(':
        case ')':
        case ';':
        case '{':
        case '}':
        case '&':
        case ',':
            cur = cur->next = calloc(1, sizeof(Token));
            cur->kind = TK_PUNCT;
            cur->loc = p;
            cur->len = 1;
            p++;
            continue;
            break;
        default:
            break;
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
        fprintf(stderr, "<%s>:unexpected character: '%c'\n",__func__ ,*p);
        exit(1);
    }
    cur = cur->next = calloc(1, sizeof(Token));
    cur->kind = TK_EOF;
    cur->loc = p;
    for(Token* tmp=head.next;tmp->kind!=TK_EOF;tmp=tmp->next){
        if(isKeyword(tmp)){
            tmp->kind=TK_KEYWORD;
        }
    }
    return head.next;
}