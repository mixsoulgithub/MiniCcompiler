#include "mini.h"

int count=0;
static void codeGen_main(ASTnode *node);

//align memory address up to 8*k .
static int align(int n, int align){
    return (n+align-1)/align * align;
}

static void gen_addr(ASTnode *node){
    switch (node->kind)
    {
    case ND_VAR:
        printf("  lea %d(%%rbp), %%rax\n", -node->var->offset);
        break;
    case ND_DEREF:
        codeGen_main(node->right);
        break;
    default:
        fprintf(stderr,"invalid node kind in gen_addr\n");
        break;
    }
    return;
}

static void codeGen_main(ASTnode *node){

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
    case ND_ADDR:
        gen_addr(node->right);
        break;
    case ND_DEREF:
        codeGen_main(node->right);
        printf("  mov (%%rax), %%rax\n");
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
        if(node->left!=NULL){
            codeGen_main(node->left);
        }
        printf("  jmp .L.return\n");
        break;
    case ND_BLOCK:
        for(ASTnode *n=node->body;n;n=n->next){
            codeGen_main(n);
        }
        break;
    case ND_IF:
        codeGen_main(node->cond);
        printf("  cmp $0, %%rax\n");
        if(node->els){
            printf("  je .L.else%d\n",count);//rax is 0, jump to else.
            codeGen_main(node->then);
            printf("  jmp .L.end%d\n",count);

            printf(".L.else%d:\n",count);
            codeGen_main(node->els);
            printf(".L.end%d:\n",count);
        }else{
            printf("  je .L.end%d\n",count);
            codeGen_main(node->then);
            printf(".L.end%d:\n",count);
        }
        count++;
        break;
    case ND_FOR:
        if(node->init){
            codeGen_main(node->init);
        }
        printf(".L.begin%d:\n",count);
        if(node->cond){
            codeGen_main(node->cond);
            printf("  cmp $0, %%rax\n");
            printf("  je .L.end%d\n",count);
        }
        codeGen_main(node->body);
        if(node->inc){
            codeGen_main(node->inc);
        }
        printf("  jmp .L.begin%d\n",count);
        printf(".L.end%d:\n",count);
        count++;
        break;
    case ND_DEC:
        for(ASTnode *n=node->body;n;n=n->next){
            codeGen_main(n);
        }
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