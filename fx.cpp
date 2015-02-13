#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstdio>
#include "fx.h"

int Fxstream::checkFormat(boost::iostreams::filtering_istream& in, std::istream& file, read_type& t, bool gzip, bool bzip2) {
    try {
#ifdef HAVE_LIBZ
        if(gzip) {
          in.push(boost::iostreams::gzip_decompressor());
        }
#endif
#ifdef HAVE_LIBBZ2 
	else if (bzip2) {
          in.push(boost::iostreams::bzip2_decompressor());
        }
#endif
        in.push(file);

        char c = in.peek();
        if(c == '>') {
            //std::cerr << "looks like a fasta file"<<std::endl;
            t = FASTA;
        } else if( c == '@') {
            //std::cerr << "looks like a fastq file"<<std::endl;
            t = FASTQ;
        } else {
            fprintf(stderr, "bad character found: %c\n", c);
            return 1;
        }
    }
    catch(const boost::iostreams::gzip_error& e) {
        fprintf(stderr, "%s\n", e.what() );
    } catch(const boost::iostreams::bzip2_error& e) {
        fprintf(stderr, "%s\n", e.what() );
    }
    return 0;
}

int Fxstream::open(const char * file1, const char * file2, bool interleaved, bool gzip, bool bzip2) {
    std::cerr << file1 <<" "<<interleaved <<" " << gzip << " "<< bzip2<<std::endl;
    in1.open(file1, std::ios_base::in | std::ios_base::binary);
    //std::ifstream file(argv[1], std::ios_base::in | std::ios_base::binary);

    if(!in1.good()) {
        fprintf(stderr, "failed to open file for mate1\n");
        return 1;
    } else if(checkFormat(f1, in1, t1, gzip, bzip2)) {
        fprintf(stderr, "The file %s does not look like either fasta or fastq\n", file1);
        return 1;
    }

    if(file2 != NULL && interleaved) {
        fprintf(stderr, "A second file cannot be given if the interleaved flag is set\n");
        return 1;
    } else if (file2 != NULL) {
        in2.open(file2, std::ios_base::in | std::ios_base::binary);
        if(!in2.good()) {
            fprintf(stderr, "failed to open file for mate2\n");
            return 1;
        } else {
            if(checkFormat(f2, in2, t2, gzip, bzip2)) {
                fprintf(stderr, "The file %s does not look like either fasta or fastq\n", file2);
                return 1;
            }

            if( t1 != t2) {
                fprintf(stderr, "both files must be either fasta or fastq\n");
                return 1;
            }
        }
    }
    this->interleaved = interleaved;
    return 0;
}

int Fxstream::close() {
    in1.close();
    if(in2.is_open()) {
        in2.close();
    }
    return 0;
}

int Fxstream::readFastaRecord(Fx& read, std::istream& input) {
    // assume that we enter this funcion always pointing at the next
    // start of a fasta record. The first line will therefore be the
    // header line
    if(input.fail()) {
      fprintf(stderr, "Internal Logic error: %s %d\n", __FILE__, __LINE__);
        return 2;
    } else if(input.eof()) {
        return 0;
    }

    input.get();
    input >> read.name;  // first word from the current line

    std::getline(input, read.comment);

    // peek the begining of the next line. it should not be the '>' char
    // if this is a valid fasta file
    if(input.peek() == '>') {
        fprintf(stderr, "malformed fasta record\n");
        return 3;
    }
    do {
        std::string tmp;
        std::getline(input, tmp);
        read.seq += tmp;
    } while (input.good() && input.peek() != '>');

    read.len = static_cast<int>(read.seq.size());

    return 0;
}

int Fxstream::readFastqRecord(Fx& read, std::istream& input) {
    // assume that we enter this funcion always pointing at the next
    // start of a fastq record. The first line will therefore be the
    // header line
    if(!input.good()) {
        return 1;
    }

    // waste the '@' char
    input.get();
    input >> read.name;  // first word from the current line

    std::getline(input, read.comment);

    // seq line
    std::getline(input, read.seq);
    read.len = static_cast<int>(read.seq.size());

    // waste line
    std::string tmp;
    std::getline(input, tmp);

    // quality line
    std::getline(input, read.qual);

    return 0;

}
//TODO: BSD grep has support for zip, bzip2 and xz files out of the box
//http://svnweb.freebsd.org/base/head/usr.bin/grep/file.c?view=markup
int Fxstream::read( Fx& read1, Fx& read2) {

    if(f1.eof()) {
        return 1;
    }
    if(t1 == FASTA) {
        if(readFastaRecord(read1, f1)) {
            return 1;
        }
        if(this->interleaved) {
            if(readFastaRecord(read2, f1)) {
                return 1;
            }
        } else if(in2.is_open() && in2.good()) {
            if(readFastaRecord(read2, f2)) {
                return 1;
            }
        }
    } else if( t1 == FASTQ) {
        if(readFastqRecord(read1, f1)) {
            return 1;
        }
        if(this->interleaved) {
            if(readFastqRecord(read2, f1)) {
                return 1;
            }
        } else if(in2.is_open() && in2.good()) {
            if(readFastqRecord(read2, f2)) {
                return 1;
            }
        }
    } else {
        // this should never happen as this kind of error should be caught previously
        fprintf(stderr, "Internal logic error occurred: %s %d\n", __FILE__, __LINE__);
        return 1;
    }
    return 0;
}

int Fxstream::read(ReadPair& pair) {
    return this->read(pair.first, pair.second);
}


#ifdef TEST_FXREAD_MAIN
int main(int argc, char * argv[]) {
    Fxstream stream;
    int state;
    if(argc == 3) {
        state = stream.open(argv[1], argv[2], false);
    } else if(argc == 2) {
        state = stream.open(argv[1], NULL, false);
    }
    if(state != 0) {
        fprintf(stderr, "Failed to open the stream\n");
        stream.close();
        return 1;
    }
    ReadPair pair;
    //Fx read1, read2;
    while(stream.read(pair.first, pair.second) == 0) {
        //std::cout << read1 << read2;
        std::cout << pair;
        pair.clear();
        //read1.clear();
        //read2.clear();
    }
    stream.close();
    return 0;
}
#endif
