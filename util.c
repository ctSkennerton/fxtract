#include "util.h"

void reverseComplement(char * str, int length) {
    int i, c0, c1;
    for (i = 0; i < length >> 1; ++i)
    {
        c0 = comp_tab[(int)str[i]];
        c1 = comp_tab[(int)str[length - 1 - i]];
        str[i] = c1;
        str[length - 1 - i] = c0;
    }
    if (length & 1)
    {
        str[length >> 1] = comp_tab[(int)str[length >> 1]];
    }
}
