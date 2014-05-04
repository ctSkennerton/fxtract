#include "util.h"

void reverseComplement(std::string& str) {
    int i, c0, c1;
    int l = static_cast<int>(str.size());
    for (i = 0; i < l >> 1; ++i)
    {
        c0 = comp_tab[(int)str[i]];
        c1 = comp_tab[(int)str[l - 1 - i]];
        str[i] = c1;
        str[l - 1 - i] = c0;
    }
    if (l & 1)
    {
        str[l >> 1] = comp_tab[(int)str[l >> 1]];
    }
}
