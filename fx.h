#ifndef _FX_H_
#define _FX_H_ 1
#include <cstring>
#include "kseq.h"

struct Fx {
  char * name;
  char * seq;
  char * qual;
  int len;

  Fx(){
    name = NULL;
    seq = NULL;
    qual = NULL;
    len = 0;
  };
  ~Fx() {
    free(name);
    free(seq);
    free(qual);
  }
  size_t size() {
    return len;
  }
  size_t length() {
    return size();
  }
  bool isFasta() {
    return qual == NULL;
  }
  int puts(FILE * out);
};

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

KSEQ_INIT(int, read);

struct Fxstream {
    kseq_t * seq1;
    kseq_t * seq2;
    int      fd1;
    int      fd2;
    bool     interleaved;

    Fxstream(){}
    int open(const char * file1, const char * file2, bool interleaved);
    int close();
    int read (Fx ** read1, Fx ** read2);

};

#endif
