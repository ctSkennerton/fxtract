#include <stdlib.h>
#include "fx.h"
char comp_tab[] = {
    0,   1,	2,	 3,	  4,   5,	6,	 7,	  8,   9,  10,	11,	 12,  13,  14,	15,
    16,  17,  18,	19,	 20,  21,  22,	23,	 24,  25,  26,	27,	 28,  29,  30,	31,
    32,  33,  34,	35,	 36,  37,  38,	39,	 40,  41,  42,	43,	 44,  45,  46,	47,
    48,  49,  50,	51,	 52,  53,  54,	55,	 56,  57,  58,	59,	 60,  61,  62,	63,
    64, 'T', 'V', 'G', 'H', 'E', 'F', 'C', 'D', 'I', 'J', 'M', 'L', 'K', 'N', 'O',
	'P', 'Q', 'Y', 'S', 'A', 'A', 'B', 'W', 'X', 'R', 'Z',	91,	 92,  93,  94,	95,
    64, 't', 'v', 'g', 'h', 'e', 'f', 'c', 'd', 'i', 'j', 'm', 'l', 'k', 'n', 'o',
	'p', 'q', 'y', 's', 'a', 'a', 'b', 'w', 'x', 'r', 'z', 123, 124, 125, 126, 127
};

void reverseComplement(sds str)
{

	int l = sdslen(str);
    int i, c0, c1;
    for (i = 0; i < l>>1; ++i) 
    {
        c0 = comp_tab[(int)str[i]];
        c1 = comp_tab[(int)str[l - 1 - i]];
        str[i] = c1;
        str[l - 1 - i] = c0;
    }
    if (l&1) 
    {
        str[l>>1] = comp_tab[(int)str[l>>1]];
    }
}

Fx * fx_new(char * d) {
    Fx * fx = malloc(sizeof(Fx));
    fx->data = sdsnew(d);
    return fx;
}

void fx_free(Fx * fx) {
    sdsfree(fx->data);
    free(fx);
}

