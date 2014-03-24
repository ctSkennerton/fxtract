#ifndef _FX_H_
#define _FX_H_ 1
#include <stdbool.h>
#include "sds/sds.h"
#include "kseq.h"

struct _Fxstream;
typedef struct _Fxstream Fxstream;
/*typedef struct {
    kseq_t * seq1; // array of input files one for each mate in the pair
    kseq_t * seq2; // array of input files one for each mate in the pair
    int      fd1;
    int      fd2;
    bool     interleaved;
} Fxstream;*/

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
 * if it is a fasta record then qual_start will equal -1
 */

typedef struct {
    sds name;
    sds seq;
    sds qual;
} Fx;


Fx * fx_new              ();
Fx * fx_new2             (char * d);
void fx_delete           (Fx * fx);
int  fx_len              (Fx * fx);
bool fx_isfasta          (Fx * fx);
//void fx_repr             (Fx * fx, sds * reprString);
void fx_tofa             (Fx * fx, sds * out);
void fx_tofq             (Fx * fx, sds * out);
int  fx_fputs            (Fx * fx, FILE * out);

/*
 * file1:       File name containing reads from the first mate in the pair
 * file2:       File name containing reads from the second mate in the pair
 *              set to NULL if only single ended reads or if the file is
 *              interleaved
 * interleaved: Set file1 to be interleaved, meaning that both pairs are
 *              sequential in the file
 *
 * returns:     Pointer to a newly allocated Fxstream that must be deleted with
 *              fxtream_close
 */
Fxstream * fxstream_open   (const char * file1, const char * file2, bool interleaved);
int        fxstream_read   (Fxstream * stream, Fx * read1, Fx * read2);
void       fxstream_close  (Fxstream * stream);

#endif
