#include "mini.h"

LocalVar *locals;
Function* functions;

static ASTnode* new_numnode(int num, ASTnode *left, ASTnode *right);

//return NULL if can't find.
LocalVar *find_lvar(char *name, int len){
    for(Scope* cur=functions->scope;cur;cur=cur->before){
        for(LocalVar *var = cur->locals; var; var = var->next){
            if(strlen(var->name) == len && !memcmp(var->name, name, len)){
                return var;
            }
        }
    }
    return NULL;
}
Function *find_func(char *name, int len){
    for(Function *func = functions; func; func = func->next){
        if(strlen(func->name) == len && !memcmp(func->name, name, len)){
            return func;
        }
    }
    return NULL;
}

//1 is equal
int type_equal(Type *ty1, Type *ty2){
    if(ty1==NULL||ty2==NULL)
        fprintf(stderr,"<%s>: type can't be NULL",__func__);
    if(ty1->tykind==TY_POINTER&&ty2->tykind==TY_POINTER){
        Type*ltmp=ty1->base;
        Type*rtmp=ty2->base;
        while(ltmp->tykind==TY_POINTER&&rtmp->tykind==TY_POINTER){
            ltmp=ltmp->base;
            rtmp=rtmp->base;
        }
        if(ltmp->tykind==rtmp->tykind){
            return 1;
        }else{
            fprintf(stderr,"<%s>:pointer layer diffrent.\n",__func__);
        }
    }else if(ty1->tykind==ty2->tykind){
        return 1;
    }else{
        return 0;
    }

    // return 1;
    
}

//1 is equal
int equal(Token *tok, char *str){
    return strlen(str) == tok->len && !memcmp(tok->loc, str, tok->len);
}


//skip string "op".
void skip(Token **tok_addr, char *op){
    Token *tok = *tok_addr;
    if(equal(tok, op)){
        *tok_addr = tok->next;
        return;
    }
    fprintf(stderr, "<skip>: want \"%s\", but unexpected token %s in \n",op,tok->loc);
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

//p+1 means different when p is int or pointer.
static ASTnode* new_addnode(ASTnode *left, ASTnode* right){
    //calculate type of node one time, or at real time? don't worry, if subnode has they type, <calcu_type> will
    //ensure avoidance of useless recursive by checking if node is null or already has type.
    Type* ltype=getNodeType(left);
    Type* rtype=getNodeType(right);
    if(ltype->tykind==TY_INT&&rtype->tykind==TY_INT){
        return new_node(ND_ADD,left,right);
    }else if(ltype->tykind==TY_POINTER && rtype->tykind==TY_INT){
        return new_node(ND_ADD,left, new_node(ND_MUL,right,new_numnode(8,NULL,NULL)));//p+1 is address +8 acctually.
    }else if(ltype->tykind==TY_INT&&rtype->tykind==TY_POINTER){
        //exchage to ensure father node to be pointer.because add inherit left node's type.
        return new_node(ND_ADD,new_node(ND_MUL,left,new_numnode(8,NULL,NULL)),right);
    }
    fprintf(stderr,"<%s>: only support pointer+int, int+int, int+pointer",__func__);
}
static ASTnode* new_subnode(ASTnode *left, ASTnode* right){

    Type* ltype=getNodeType(left);
    Type* rtype=getNodeType(right);
    if(ltype->tykind==TY_INT&&rtype->tykind==TY_INT){
        return new_node(ND_SUB,left,right);
    }else if(ltype->tykind==TY_POINTER && rtype->tykind==TY_INT){
        return new_node(ND_SUB,left, new_node(ND_MUL,right,new_numnode(8,NULL,NULL)));//p+1 is address +8 acctually.
    }else if(ltype->tykind==TY_POINTER&&rtype->tykind==TY_POINTER){
        Type*ltmp=ltype->base;
        Type*rtmp=rtype->base;
        while(ltmp->tykind==TY_POINTER&&rtmp->tykind==TY_POINTER){
            ltmp=ltmp->base;
            rtmp=rtmp->base;
        }
        if(ltmp->tykind==rtmp->tykind){
            ASTnode *tmp=new_node(ND_DIV,new_node(ND_SUB,left,right),new_numnode(8,NULL,NULL));
            // ptr-ptr returns a int type num. but what for subnode's type? 
            //we don't care until use, just calcu_type at that time. 
            tmp->type=ty_int;
            return tmp;
        }else{
            fprintf(stderr,"<%s>:when pointer-pointer, pointers must at same kind and layer",__func__);
        }
    }
    fprintf(stderr,"<%s>: only support pointer-int, int-int, pointer-pointer",__func__);
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
    }else{
        fprintf(stderr,"undefined variable at file %s, line %d",__FILE__,__LINE__);
    }
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
static ASTnode* new_fornode(ASTnode *init, ASTnode *cond, ASTnode *inc, ASTnode *body){
    ASTnode *node = new_node(ND_FOR, NULL, NULL);
    node->init = init;
    node->cond = cond;
    node->inc = inc;
    node->body = body;
    return node;
}
//id
static ASTnode* new_declare_lvarnode(Token **tok_addr,Type* type){
    
    ASTnode *node;
    if(find_lvar((*tok_addr)->loc,(*tok_addr)->len)==NULL){
        
        //initialize local variable
        LocalVar *var=calloc(1, sizeof(LocalVar));
        var->name = strndup((*tok_addr)->loc, (*tok_addr)->len);//strndup is not standard, but it's in glibc,'\0' is included automatically.
        var->next = functions->scope->locals;
        var->type=type;//!!!
        var->offset = functions->scope->locals ? (functions->scope->locals->offset) + 8 : 8; 
        //add var to scope
        functions->scope->locals = var;

        //create node and bind var to node
        node=new_lvarnode((*tok_addr)->loc,(*tok_addr)->len,NULL,NULL);
        node->var=var;
        node->type=type;
        *tok_addr=(*tok_addr)->next;
        return node;
    }else{
        fprintf(stderr,"redefination of variable in file %s, line %d",__FILE__,__LINE__);
        exit(1);
    }
}

static ASTnode* file(Token **tok_addr);
static ASTnode* function_declare(Token **tok_addr);
static ASTnode* block(Token **tok_addr);
static ASTnode* sentence(Token **tok_addr);
static ASTnode* assign(Token **tok_addr);
static ASTnode* equation(Token **tok_addr);
static ASTnode* relational(Token **tok_addr);
static ASTnode* add(Token **tok_addr);
static ASTnode* mul(Token **tok_addr);
static ASTnode* primary(Token **tok_addr);
static ASTnode* unary(Token **tok_addr);
//tok is the first token of the expression.

//file = function_declare*
static ASTnode* file(Token **tok_addr){
    ASTnode* node=function_declare(tok_addr);
    while ((*tok_addr)->kind!=TK_EOF)
    {
        node->next=function_declare(tok_addr);
    }
    
    return node;
}

//function_declare=type "*"* id "()" block;
static ASTnode* function_declare(Token **tok_addr){
    Type *basetype= getbasetype(*tok_addr);
    *tok_addr=(*tok_addr)->next;
    if(find_func((*tok_addr)->loc,(*tok_addr)->len)){
        fprintf(stderr,"redefination of function in file %s, line %d",__FILE__,__LINE__);
        exit(1);
    }
    Type * final=basetype;
    for(;equal(*tok_addr,"*")!=0;skip(tok_addr,"*")){
        final=point_to(final);
    }
    ASTnode* node=new_node(ND_FUNDEF,NULL,NULL);

    //initialize function
    Function* func=calloc(1,sizeof(Function));
    func->name=strndup((*tok_addr)->loc,(*tok_addr)->len);
    func->type =node->type=final;
    func->scope=calloc(1,sizeof(Scope));

    //binding node to function
    node->func=func;

    //update function list
    func->next=functions;
    functions=func;

    *tok_addr=(*tok_addr)->next;
    skip(tok_addr,"(");
    skip(tok_addr,")");
    node->body=block(tok_addr);
    return node;
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
        // fprintf(stderr,"next sentence first token is %s\n",tok->loc);
    }
    tok = tok->next;
    *tok_addr = tok;
    return new_blocknode(head.next);
}

//sentence = ";" | "return" ";" | "return" assgin ";"| assgin ";" | block 
//          | "if" "(" equation ")" sentence ("else" sentence)?
//          | "for" "(" assgin ";" equation ";" assgin ")" sentence
//          | "while" "(" equation ")" sentence
//          |types "*"* id_d (= assign)? ("," "*"* id_d (=assign)?)* ;
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

    if(tok->kind == TK_KEYWORD){
        if(equal(*tok_addr, "return")){
            skip(tok_addr,"return");
            // fprintf(stderr,"now token in %s",(*tok_addr)->loc);
            ASTnode *node = new_node(ND_RETURN, assign(tok_addr), NULL);
            // if(!type_equal(getNodeType(node->left),functions->type)){
            //     fprintf(stderr,"<%s>: return type unmatch in %s",__func__,functions->name);
            // }
            if(!type_equal(getNodeType(node->left),functions->type)){
                fprintf(stderr,"<%s>: return type unmatch in %s \n",__func__,functions->name);
            }
            skip(tok_addr, ";");
            return node;
        }
        if(equal(tok, "if")){
            skip(tok_addr,"if");
            skip(tok_addr, "(");
            ASTnode *cond = equation(tok_addr);
            skip(tok_addr, ")");
            ASTnode *then = sentence(tok_addr);
            if(equal(*tok_addr, "else")){
                skip(tok_addr,"else");
                ASTnode *els = sentence(tok_addr);
                return new_ifnode(cond, then, els);
            }
            return new_ifnode(cond, then, NULL);
        }
        if(equal(tok, "for")){
            skip(tok_addr,"for");
            ASTnode *init, *cond, *inc;
            init = cond = inc = NULL;
            skip(tok_addr, "(");

            if(!equal(*tok_addr, ";")){
                init = assign(tok_addr);//will be NULL if there is no initialization.
            }
            skip(tok_addr, ";");

            if(!equal(*tok_addr, ";")){
                cond = equation(tok_addr);
            }
            skip(tok_addr, ";");

            if(!equal(*tok_addr, ")")){
                inc = assign(tok_addr);
            }
            skip(tok_addr, ")");

            ASTnode *body = sentence(tok_addr);
            return new_fornode(init, cond, inc, body);
        }
        if(equal(tok, "while")){
            tok = tok->next;
            *tok_addr = tok;
            skip(tok_addr, "(");
            ASTnode *cond = equation(tok_addr);
            skip(tok_addr, ")");
            ASTnode *body = sentence(tok_addr);
            return new_fornode(NULL, cond, NULL, body);
        }
    }
    
    if(isBaseType(*tok_addr)){
        Type *basetype= getbasetype(*tok_addr);
        *tok_addr=(*tok_addr)->next;
        Type * final=basetype;
        for(;equal(*tok_addr,"*")!=0;skip(tok_addr,"*")){
            final=point_to(final);
        }
        // skip(tok_addr,"int");
        ASTnode head={};
        ASTnode *cur=&head;

        ASTnode* node=new_declare_lvarnode(tok_addr,final);
        if(equal(*tok_addr,"=")){
            skip(tok_addr,"=");
            node=new_node(ND_ASSIGN,node,assign(tok_addr));
        }
        cur=cur->next=node;

        for(;equal(*tok_addr,",");){
            skip(tok_addr,",");

            Type * final=basetype;
            for(;equal(*tok_addr,"*")!=0;skip(tok_addr,"*")){
                final=point_to(final);
            }
            ASTnode* node=new_declare_lvarnode(tok_addr,final);

            if(equal(*tok_addr,"=")){
                // fprintf(stderr,"\n try to assign!\n");
                skip(tok_addr,"=");
                node=new_node(ND_ASSIGN,node,assign(tok_addr));
            }
            cur=cur->next=node;
        }
        skip(tok_addr,";");
        ASTnode *ans=new_node(ND_DEC,NULL,NULL);
        ans->body=head.next;
        return ans;
    }

    ASTnode *node = assign(tok_addr);
    skip(tok_addr, ";");
    return node;
}


//assgin = equation ("=" assgin)?
static ASTnode* assign(Token **tok_addr){
    ASTnode *node = equation(tok_addr);
    Token *tok = *tok_addr;
    if(tok->kind == TK_PUNCT && *tok->loc == '='){
        tok = tok->next;
        *tok_addr = tok;
        return new_node(ND_ASSIGN, node, assign(tok_addr));//assgin is right-associative.
    }
    return node;
}

//equation = relational ("==" relational | "!=" relational)*
static ASTnode* equation(Token **tok_addr){
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
            node = new_addnode(node, rhs);
        }else{
            node = new_subnode(node, rhs);//in this layer, node becomes left, so left-associative is guaranteed.
        }
    }
    return node;
}

//mul = unary ("*" unary | "/" unary)*
static ASTnode* mul(Token **tok_addr){
    ASTnode *node = unary(tok_addr);
    Token *tok= *tok_addr;

    for(;;){
        if(equal(tok,"/")){
            skip(tok_addr,"/");
            return new_node(ND_DIV, node, unary(tok_addr));
        }else if(equal(tok,"*")){
            skip(tok_addr,"*");
            return new_node(ND_MUL, node, unary(tok_addr));
        }else{
            // fprintf(stderr,"<mul> error: need / or * \n");
            // exit(1);
            break;
        }
    }
    return node;
}

//unary = ("+"|"-"|"*"|"&") unary
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
        ASTnode *node =new_node(ND_NEG, unary(tok_addr), NULL);
        tok=*tok_addr;
        return node;
    }
    if(*tok->loc=='&'){
        *tok_addr=tok->next;
        ASTnode *node =new_node(ND_ADDR, unary(tok_addr), NULL);
        tok=*tok_addr;
        return node;
    }
    if(*tok->loc=='*'){
        *tok_addr=tok->next;
        ASTnode *node =new_node(ND_DEREF, unary(tok_addr), NULL);
        tok=*tok_addr;
        return node;
    }
    return primary(tok_addr);
}


//primary = num | "(" equiality ")" | ident ("(" ")")?
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
        ASTnode *node = equation(tok_addr);
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
        if(equal((*tok_addr)->next,"(")){
            ASTnode* node=new_node(ND_FUNCALL,NULL,NULL);

            //binding node to function
            node->func=find_func((*tok_addr)->loc,(*tok_addr)->len);
            //function args node generate and type check
            
            *tok_addr=(*tok_addr)->next;
            skip(tok_addr,"(");
            skip(tok_addr,")");
            return node;
        }
        ASTnode *node = new_lvarnode(tok->loc,tok->len, NULL, NULL);
        *tok_addr=(*tok_addr)->next;
        return node;
    }
    fprintf(stderr, "<primary> unexpected token %s\n", (*tok_addr)->loc);
    exit(1);
}

ASTnode *ASTgen(Token *tok){
    return file(&tok);
}