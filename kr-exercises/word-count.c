#include <stdio.h>

#define IN 1
#define OUT 0

#define WORD_LEN 8

int main() {
    char c, w[WORD_LEN];
    int nl, nw, nc, state, idx;
    nl = nw = nc = 0;
    state = OUT;
    idx = 0;
    while ((c = getchar()) != EOF) {
        nc++;
        if (c == '\n') nl++;
        if (c == ' ' || c == '\t' || c == '\n') {
            if (state == IN) {
                w[idx] = '\0';
                printf("%s\n", w);
                state = OUT;
                idx = 0;
            }
        } else {
            if (idx < WORD_LEN - 1)
                w[idx++] = c;
            if (state == OUT) {
                nw++;
                state = IN;
            }
        }
    }
    if (state == IN) {
        w[idx] = '\0';
        printf("%s\n", w);
    }
    printf("%d %d %d\n", nl, nw, nc);
    return 0;
}
