#ifndef _FX_H_
#define _FX_H_ 1
#include "sds/sds.h"
/* Representation of a fasta/fastq record
 * All of the string data is defined in a single block called `data'
 * with references to the beginning of the header, sequence and quality
 * data witin that block.  The members header_len, seq_len, qual_len give the
 * length of the associated data
 * 
 * HEADERSEQUENCEQUALITY
 * |     |       |
 * |     |       qual_start
 * |     seq_start
 * header_start
 *
 * if it is a fasta record then qual_start will equal NULL
 */
typedef struct {
    int    header_start;
    int    seq_start;
    int    qual_start;
    int    header_len;
    int    seq_len;
    int    qual_len;
    sds    data;
} Fx;

/* Same as Fx however without the string data.  Only contains the references
 * to another buffer stored externally
 */
typedef struct {
    int    header_start;
    int    seq_start;
    int    qual_start;
    int    header_len;
    int    seq_len;
    int    qual_len;
} Fxref;



Fx * fx_new(char * d);
void fx_free(Fx * fx);

#endif
