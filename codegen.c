#include "mini.h"
static void gen_addr(ASTnode *node){
    if(node->kind == ND_VAR){
        printf("  lea %d(%%rbp), %%rax\n", -node->var->offset);
        //now rax is the address of the variable.
    }else{
        fprintf(stderr,"invalid node kind in gen_addr\n");
        return;
    }
    return;
}

//align(5,8)=8, align(8,8)=8, align(9,8)=16. to align memory address.
static int align(int n, int align){
    return (n+align-1)/align * align;
}

static void codeGen_main(ASTnode *node){
    // if(node->tok->kind == TK_NUM && node->left == NULL && node->right == NULL){
    //     printf("  mov $%d, %%rax\n", node->tok->val);
    //     return;
    // }
    // if(node->tok->kind == TK_PUNCT && *node->tok->loc == '-'&& node->left == NULL){
    //     codeGen_main(node->right);
    //     printf("  neg %%rax\n");
    //     return;
    // }
    // if(node->tok->kind == TK_PUNCT && *node->tok->loc == '+'&& node->left == NULL){
    //     codeGen_main(node->right);
    //     return;
    // }
    if(node->right!=NULL&&node->left!=NULL){//handle normal codition together in case stack imbalance.
        codeGen_main(node->right);
        printf("  push %%rax\n");
        codeGen_main(node->left);
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
        codeGen_main(node->right);
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
    case ND_EMPTY:
        break;
    case ND_ASSIGN:
        gen_addr(node->left);
        printf("  push %%rax\n");
        codeGen_main(node->right);
        printf("  pop %%rdi\n");
        printf("  mov %%rax, (%%rdi)\n");
        break;
    case ND_VAR:
        gen_addr(node);
        printf("  mov (%%rax), %%rax\n");//get value from the address.
        break;
    case ND_RETURN:
        codeGen_main(node->left);
        printf("  jmp .L.return\n");
        break;
    default:
        fprintf(stderr,"invalid node kind in codeGen_main\n");
        return;
    }
    return;
}

void codeGen(ASTnode *node){
    printf("  .globl main\n");
    printf("main:\n");
    printf("  push %%rbp\n");//save the base pointer.
    printf("  mov %%rsp, %%rbp\n");//set the base pointer.
    if(locals){
        printf("  sub $%d, %%rsp\n",align(locals->offset, 16));
    }
    for(;node;node=node->next){
        codeGen_main(node);
    }
    printf(".L.return:\n");
    printf("  mov %%rbp, %%rsp\n");//restore the stack pointer.
    printf("  pop %%rbp\n");//restore the base pointer.
    printf("  ret\n");
    return;
}