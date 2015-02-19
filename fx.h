#ifndef _FX_H_
#define _FX_H_ 1
#include <string>
#include <iostream>
#include <fstream>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filter/bzip2.hpp>

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
    bool empty() {
        return name.empty() && comment.empty() && seq.empty() && qual.empty();
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
    void print(FILE * out) {
        if(!empty()) {
            if(isFasta()) {
                fprintf(out, ">%s", name.c_str());
                if(!comment.empty()) {
                    fprintf(out, "%s",comment.c_str());
                }
                fprintf(out, "\n%s\n", seq.c_str());
            } else {
                fprintf(out, "@%s", name.c_str());
                if(!comment.empty()) {
                    fprintf(out, "%s", comment.c_str());
                }
                fprintf(out, "\n%s+\n%s\n", seq.c_str(), qual.c_str());
            }
        }
    }

};

//typedef std::pair<Fx, Fx> ReadPair;

struct ReadPair {
    Fx first;
    Fx second;

    void clear() {
        first.clear();
        second.clear();
    }

    void print(FILE * out) {
        first.print(out);
        second.print(out);
    }
};



inline std::ostream& operator<<(std::ostream& out, Fx& mate) {
    if(mate.empty()) {
        return out;
    }
    if(mate.isFasta()) {
        out << ">"<<mate.name;
        if(!mate.comment.empty())
            out<<mate.comment;
        out<<"\n"<<mate.seq<<"\n";
    } else {
        out << "@"<<mate.name;
        if(!mate.comment.empty())
            out<<mate.comment;
        out<<"\n"<<mate.seq<<"\n+\n"<<mate.qual<<"\n";
    }
    return out;
}

inline std::ostream& operator<<(std::ostream& out, ReadPair& pair) {
    out << pair.first <<pair.second;
    return out;
}
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
    std::ifstream                       in1;
    std::ifstream                       in2;
    boost::iostreams::filtering_istream f1;
    boost::iostreams::filtering_istream f2;
    read_type                           t1;
    read_type                           t2;
    bool                                interleaved;

    Fxstream(){}
    int open(const char * file1, const char * file2, bool interleaved, bool gzip, bool bzip2);
    int close();
    int read (Fx& read1, Fx& read2);
    int read (ReadPair& pair);

    private:
    int checkFormat(boost::iostreams::filtering_istream& in, std::istream& file, read_type& t, bool gzip, bool bzip2);
    int readFastaRecord(Fx& read, std::istream& input);
    int readFastqRecord(Fx& read, std::istream& input);

};

#endif
