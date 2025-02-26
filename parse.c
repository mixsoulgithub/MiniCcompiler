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

static int equal(Token *tok, char *node_tok){
    return strlen(node_tok) == tok->len && !memcmp(tok->loc, node_tok, tok->len);
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

static ASTnode* expr(Token **tok_addr);
static ASTnode* sentence(Token **tok_addr);
static ASTnode* assign(Token **tok_addr);
static ASTnode* equaility(Token **tok_addr);
static ASTnode* relational(Token **tok_addr);
static ASTnode* add(Token **tok_addr);
static ASTnode* mul(Token **tok_addr);
static ASTnode* primary(Token **tok_addr);
static ASTnode* unary(Token **tok_addr);
//tok is the first token of the expression.

//expr = sentence*
static ASTnode* expr(Token **tok_addr){
    Token *tok = *tok_addr;
    ASTnode head = {};
    ASTnode *node = &head;
    while(tok->kind != TK_EOF){
        node =node->next= sentence(tok_addr);
        tok = *tok_addr;
    }
    return head.next;//struct is different from pointer to struct.
}

//sentence = assgin ";" | ";"
static ASTnode* sentence(Token **tok_addr){
    Token *tok = *tok_addr;
    if(tok->kind == TK_PUNCT && *tok->loc == ';'){
        tok = tok->next;
        *tok_addr = tok;
        return new_node(ND_EMPTY, NULL, NULL);
    }
    ASTnode *node = assign(tok_addr);
    tok = *tok_addr;
    if(tok->kind == TK_PUNCT && *tok->loc == ';'){
        tok = tok->next;
        *tok_addr = tok;
        return node;
    }
    fprintf(stderr, "unexpected token when need ';'\n");
    exit(1);
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