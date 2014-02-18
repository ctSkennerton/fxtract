#include <stdio.h>
#include <zlib.h>
/* This enum defines how the pairs of reads are arranged
 * SINGLE: there are no pairs, only single reads
 * INTERLEAVED: the pairs are in the same file, one afer the other
 * PAIRED: there are two files one for each pair
 */
enum PairType {
    SINGLE,
    INTERLEAVED,
    PAIRED
};
/* This enum defines what type of data is contained in the files
 * FASTA: this is fasta data
 * FASTQ: this is fastq data
 */
enum RecordType {
    FASTA,
    FASTQ
};

/* The maximum length of the internal buffer
 * each read from the file is BUFFER_SIZE - 1
 */
#define BUFFER_SIZE 8192

struct _fxstream;
typedef _fxstream fxstream;

void fxstream_read(fxstream* s);

void fxstream_readgzip(fxstream* s);

void fxstream_readbzip2(fxstream* s);
