#ifndef _FX_H_
#define _FX_H_ 1
#include <string>
#include <iostream>
#include "kseq.hpp"


struct ReadPair {
    kseq first;
    kseq second;

    void clear() {
        first.clear();
        second.clear();
    }

    void print(FILE * out) {
        first.print(out);
        second.print(out);
    }
};

inline std::ostream& operator<<(std::ostream& out, ReadPair& pair) {
    out << pair.first <<pair.second;
    return out;
}

struct Fxstream {

    Fxstream();
    ~Fxstream();
    int open(const char * file1, const char * file2, bool interleaved);
    int close();
    int read (kseq& read1, kseq& read2);
    int read (ReadPair& pair);
  private:
    FunctorZlib gzr;
    kstream<gzFile, FunctorZlib> * ks1;
    kstream<gzFile, FunctorZlib> * ks2;
    gzFile in1;
    gzFile in2;
    bool   interleaved;

};

#endif
