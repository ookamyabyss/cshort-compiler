#include <stdio.h>
#include "lexer.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s <arquivo>\n", argv[0]);
        return 1;
    }

    reinicia_lexer(argv[1]);

    TOKEN tk;
    do {
        tk = proximo_token();

        switch (tk.cat) {
            case ID:
                printf("Token ID: %s\n", tk.lexema);
                break;
            case INTCON:
                printf("Token INT: %s\n", tk.lexema);
                break;
            case SN:
                if (tk.atributo.codigo == TK_EOF) {
                    printf("Token EOF\n");
                } else {
                    printf("Token SN: '%c' (%d)\n", tk.atributo.codigo, tk.atributo.codigo);
                }
                break;
            default:
                printf("Token desconhecido\n");
        }
    } while (tk.cat != SN || tk.atributo.codigo != TK_EOF);

    return 0;
}
