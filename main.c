#include "mini.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr,"Usage: %s <source code string>\n", argv[0]);
        return 1;
    }
    Token *tok = Tokenlize(argv[1]);
    if (tok->kind == TK_EOF) {
        fprintf(stderr, "no input\n");
        return 1;
    }
    ASTnode *node = ASTgen(tok);
    codeGen(node);
    return 0;
}