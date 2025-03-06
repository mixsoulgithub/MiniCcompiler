#include "mini.h"
void calcu_type(ASTnode *node);

Type * ty_int = &(Type){.base=NULL,.tykind=TY_INT};//anonymous type_int.

bool is_int(Type *ty){
    return ty->tykind==TY_INT;
}
 
Type * point_to(Type*ty){
    Type * new=calloc(1, sizeof(Type));
    new->base=ty;
    new->tykind=TY_POINTER;
    return new;
}

Type* getNodeType(ASTnode*node){
    if(!node){
        fprintf(stderr,"<%s>: node can't be NULL",__func__);
        exit(1);
    }else{
        calcu_type(node);
        return node->type;
    }
}

void calcu_type(ASTnode *node){
    if(!node||node->type)
        return;
    calcu_type(node->left);
    calcu_type(node->right);
    calcu_type(node->cond);
    calcu_type(node->inc);
    calcu_type(node->init);
    calcu_type(node->then);
    calcu_type(node->els);
    // calcu_type(node->next);//? do we need it? 
    for(ASTnode* n=node->body;n;n=n->next){
        calcu_type(n);
    }
    switch (node->kind)
    {
    case ND_ADD:
    case ND_SUB:
    case ND_MUL:
    case ND_DIV:
        node->type = node->left->type;
        return;
    case ND_NEG:
    case ND_ASSIGN:
        node->type = node->right->type;
        return;
    case ND_ADDR:
        node->type=point_to(node->right->type);
        return;
    case ND_DEREF:
        if(node->type->tykind==TY_POINTER){
            node->type=node->right->type->base;
        }else{
            fprintf(stderr,"dereference of non-pointer");
        }
        return;
    case ND_EQ:
    case ND_NE:
    case ND_LT:
    case ND_LE:
    case ND_VAR:
    case ND_NUM:
        node->type=ty_int;
    case ND_FUNCALL:
        node->type=ty_int;
        return;
        
    default:
        return;
    }
}