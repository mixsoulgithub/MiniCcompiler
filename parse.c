#include "mini.h"

LocalVar *locals;

LocalVar *find_lvar(char *name, int len){
    for(LocalVar *var = locals; var; var = var->next){
        if(strlen(var->name) == len && !memcmp(var->name, name, len)){
            return var;
        }
    }
    return NULL;
}

int equal(Token *tok, char *str){
    return strlen(str) == tok->len && !memcmp(tok->loc, str, tok->len);
}

void skip(Token **tok_addr, char *op){
    Token *tok = *tok_addr;
    if(equal(tok, op)){
        *tok_addr = tok->next;
        return;
    }
    fprintf(stderr, "unexpected token %s\n", tok->loc);
    exit(1);
}

static ASTnode* new_node(NodeKind kind, ASTnode *left, ASTnode *right){
    ASTnode *node = calloc(1, sizeof(ASTnode));
    node->kind = kind;
    // fprintf(stderr,"new a node. kind is : %d\n", node->kind);
    node->left = left;
    node->right = right;
    return node;
}

static ASTnode* new_numnode(int num, ASTnode *left, ASTnode *right){
    ASTnode *node = new_node(ND_NUM, left, right);
    node->val=num;
    return node;
}//many number nodes are created, it's a waste to use "if" to distinguish them in new_node function.

static ASTnode* new_lvarnode(char* name,int len, ASTnode *left, ASTnode *right){
    ASTnode *node = new_node(ND_VAR, left, right);

    LocalVar *var=find_lvar(name, len);
    if(var){
        node->var=var;
        return node;
    }
    //head insert makes earlier defined variable have smaller offset and closer to rbp.
    var = calloc(1, sizeof(LocalVar));
    var->name = strndup(name, len);//strndup is not standard, but it's in glibc,'\0' is included automatically.
    var->next = locals;
    var->offset = locals ? (locals->offset) + 8 : 8; 
    locals = var;
    node->var=var;
    return node;
}

static ASTnode* new_blocknode(ASTnode *body){
    ASTnode *node = new_node(ND_BLOCK, NULL, NULL);
    node->body = body;
    return node;
}

static ASTnode* new_ifnode(ASTnode *cond, ASTnode *then, ASTnode *els){
    ASTnode *node = new_node(ND_IF, NULL, NULL);
    node->cond = cond;
    node->then = then;
    node->els = els;
    return node;
}

static ASTnode* expr(Token **tok_addr);
static ASTnode* block(Token **tok_addr);
static ASTnode* sentence(Token **tok_addr);
static ASTnode* assign(Token **tok_addr);
static ASTnode* equaility(Token **tok_addr);
static ASTnode* relational(Token **tok_addr);
static ASTnode* add(Token **tok_addr);
static ASTnode* mul(Token **tok_addr);
static ASTnode* primary(Token **tok_addr);
static ASTnode* unary(Token **tok_addr);
//tok is the first token of the expression.

//expr = block
static ASTnode* expr(Token **tok_addr){
    return block(tok_addr);
}

//block= "{" sentence* "}"
static ASTnode* block(Token **tok_addr){
    Token *tok = *tok_addr;
    skip(tok_addr, "{");

    ASTnode head = {};
    ASTnode *node = &head;
    while(!(tok->kind == TK_PUNCT && *tok->loc == '}')){//lack good error message if  '}' is missing.
        node = node->next = sentence(tok_addr);
        tok = *tok_addr;
    }
    tok = tok->next;
    *tok_addr = tok;
    return new_blocknode(head.next);
}


//sentence = ";" | "return" assgin ";"| assgin ";" | block 
//          | "if" "(" equaility ")" sentence ("else" sentence)?
//reconginze keyword and terminal symbol first.
static ASTnode* sentence(Token **tok_addr){
    Token *tok = *tok_addr;

    if(tok->kind == TK_PUNCT && *tok->loc == ';'){
        tok = tok->next;
        *tok_addr = tok;
        return new_node(ND_EMPTY, NULL, NULL);
    }

    if(tok->kind == TK_PUNCT && *tok->loc == '{'){
        return block(tok_addr);
    }

    if(tok->kind == TK_ID){
        if(equal(tok, "return")){
            tok = tok->next;
            *tok_addr = tok;
            ASTnode *node = new_node(ND_RETURN, assign(tok_addr), NULL);
            skip(tok_addr, ";");
            return node;
        }
        if(equal(tok, "if")){
            tok = tok->next;
            *tok_addr = tok;
            skip(tok_addr, "(");
            ASTnode *cond = equaility(tok_addr);
            skip(tok_addr, ")");
            ASTnode *then = sentence(tok_addr);
            tok = *tok_addr;
            if(equal(tok, "else")){
                tok = tok->next;
                *tok_addr = tok;
                ASTnode *els = sentence(tok_addr);
                return new_ifnode(cond, then, els);
            }
            return new_ifnode(cond, then, NULL);
        }
    }

    ASTnode *node = assign(tok_addr);
    skip(tok_addr, ";");
    return node;
}

//assgin = equaility ("=" assgin)?
static ASTnode* assign(Token **tok_addr){
    ASTnode *node = equaility(tok_addr);
    Token *tok = *tok_addr;
    if(tok->kind == TK_PUNCT && *tok->loc == '='){
        tok = tok->next;
        *tok_addr = tok;
        return new_node(ND_ASSIGN, node, assign(tok_addr));//assgin is right-associative.
    }
    return node;
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


//primary = num | "(" equiality ")" | ident
static ASTnode* primary(Token **tok_addr){
    Token *tok = *tok_addr;
    if(tok->kind == TK_NUM){
        ASTnode *node = new_numnode(tok->val, NULL, NULL);
        tok = tok->next;
        *tok_addr = tok;
        return node;
    }

    if(tok->kind == TK_PUNCT && *tok->loc == '('){
        tok = tok->next;
        *tok_addr = tok;
        ASTnode *node = equaility(tok_addr);
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

    if(tok->kind == TK_ID){
        ASTnode *node = new_lvarnode(tok->loc,tok->len, NULL, NULL);
        tok = tok->next;
        *tok_addr = tok;
        return node;
    }
    fprintf(stderr, "unexpected token %s\n", tok->loc);
    exit(1);
}

ASTnode *ASTgen(Token *tok){
    return expr(&tok);
}