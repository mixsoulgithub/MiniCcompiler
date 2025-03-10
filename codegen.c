#include "mini.h"

int branch_count=0;
int func_count=0;
static void codeGen_main(ASTnode *node);

//align address n up to base*k .
static int align(int n, int base){
    return (n+base-1)/base * base;
}

static void gen_addr(ASTnode *node){
    switch (node->kind)
    {
    case ND_VAR:
       //offset >0 is local variable, <0 is function arguments. 
       if(node->var->offset<0){
            printf("  lea %d(%%rbp), %%rax\n", -node->var->offset + 8);//8 is the breakpoint's size.
            break;
        }else{
            printf("  lea %d(%%rbp), %%rax\n", -node->var->offset);
            break;
        }
    case ND_DEREF:
        codeGen_main(node->left);
        break;
    default:
        fprintf(stderr,"invalid node kind in gen_addr\n");
        break;
    }
    return;
}

static void codeGen_main(ASTnode *node){
    switch (node->kind)
    {
    case ND_ASSIGN:
        gen_addr(node->left);
        // codeGen_main(node->left);?
        printf("  push %%rax\n");
        codeGen_main(node->right);
        printf("  pop %%rdi\n");
        printf("  mov %%rax, (%%rdi)\n");
        return;
    default:
        break;
    }

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
        codeGen_main(node->left);
        printf("  neg %%rax\n");
        break;
    case ND_ADDR:
        gen_addr(node->left);
        break;
    case ND_DEREF:
        codeGen_main(node->left);
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
    case ND_VAR:
        gen_addr(node);
        printf("  mov (%%rax), %%rax\n");//get value from the address.
        break;
    case ND_RETURN:
        if(node->left!=NULL){
            codeGen_main(node->left);
        }
        printf("  jmp .L.return%d\n",func_count);
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
            printf("  je .L.else%d\n",branch_count);//rax is 0, jump to else.
            codeGen_main(node->then);
            printf("  jmp .L.end%d\n",branch_count);

            printf(".L.else%d:\n",branch_count);
            codeGen_main(node->els);
            printf(".L.end%d:\n",branch_count);
        }else{
            printf("  je .L.end%d\n",branch_count);
            codeGen_main(node->then);
            printf(".L.end%d:\n",branch_count);
        }
        branch_count++;
        break;
    case ND_FOR:
        if(node->init){
            codeGen_main(node->init);
        }
        printf(".L.begin%d:\n",branch_count);
        if(node->cond){
            codeGen_main(node->cond);
            printf("  cmp $0, %%rax\n");
            printf("  je .L.end%d\n",branch_count);
        }
        codeGen_main(node->body);
        if(node->inc){
            codeGen_main(node->inc);
        }
        printf("  jmp .L.begin%d\n",branch_count);
        printf(".L.end%d:\n",branch_count);
        branch_count++;
        break;
    case ND_DEC:
        for(ASTnode *n=node->body;n;n=n->next){
            codeGen_main(n);
        }
        func_count++;
        break;
    case ND_FUNCALL:
        LocalVar * cur_arg=node->func->args;
        for(ASTnode *tmp =node->init;tmp;tmp=tmp->next){
            codeGen_main(tmp);
            printf("  push %%rax\n");
            cur_arg=cur_arg->next;
        }
        printf("  mov $0, %%rax\n");
        printf("  call %s\n", node->func->name);
        printf("  add $%d, %%rsp\n",node->func->argn*8);
        return;
    case ND_FUNDEF:
        printf("  .globl %s\n",node->func->name);
        printf("%s:\n",node->func->name);
        printf("  push %%rbp\n");//save the base pointer.
        printf("  mov %%rsp, %%rbp\n");//set the base pointer.
        if(node->func){
            //frame size is 16 aligned, x86_64 ABI.
            printf("  sub $%d, %%rsp\n",align(node->func->fs, 16));
        }
        for(ASTnode* now=node->body;now;now=now->next){
            codeGen_main(now);
        }
        printf(".L.return%d:\n",func_count);
        printf("  mov %%rbp, %%rsp\n");//restore the stack pointer.
        printf("  pop %%rbp\n");//restore the base pointer.
        printf("  ret\n\n");
        func_count++;
        return;
    default:
        fprintf(stderr,"invalid node kind in codeGen_main\n");
        return;
    }
    return;
}

void codeGen(ASTnode *node){
    for(;node;node=node->next){
        codeGen_main(node);
    }
    return;
}