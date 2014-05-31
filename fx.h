#ifndef _FX_H_
#define _FX_H_ 1
#include <string>
#include <fstream>

struct Fx {
    std::string name;
    std::string comment;
    std::string seq;
    std::string qual;
    int         len;

    Fx(){
        len = 0;
    };
    ~Fx() {
    }
    size_t size() {
        return len;
    }
    size_t length() {
        return size();
    }
    bool isFasta() {
        return qual.empty();
    }
    void clear() {
        name.clear();
        comment.clear();
        seq.clear();
        qual.clear();
    }
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

struct Fxstream {

    enum read_type {
        FASTA,
        FASTQ
    };
    std::ifstream f1;
    std::ifstream f2;
    read_type     t1;
    read_type     t2;
    bool          interleaved;

    Fxstream(){}
    int open(const char * file1, const char * file2, bool interleaved);
    int close();
    int read (Fx ** read1, Fx ** read2);

    private:
    int readFastaRecord(Fx ** read, std::ifstream& input);
    int readFastqRecord(Fx ** read, std::ifstream& input);

};

#endif
