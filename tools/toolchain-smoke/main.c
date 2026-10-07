/* IMPL-0001: independently authored bootstrap probe; no Macintosh behavior. */
#include <stdio.h>

int main(void)
{
    if (puts("ClasSICK 0.97 toolchain smoke: PASS") == EOF) {
        return 1;
    }
    return 0;
}
