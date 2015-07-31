#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstdio>
#include "fx.h"

Fxstream::~Fxstream()
{
    if(ks1 != NULL)
    {
        delete ks1;
    }
    if(ks2 != NULL)
    {
        delete ks2;
    }
}

int Fxstream::open(const char * file1, const char * file2, bool interleaved) {
    in1 = gzopen(file1, "r");
    ks1 = new kstream<gzFile, FunctorZlib>(in1, gzr);

    if(file2 != NULL && interleaved) {
        fprintf(stderr, "A second file cannot be given if the interleaved flag is set\n");
        return 1;
    } else if (file2 != NULL) {
        in2 = gzopen(file2, "r");
        ks2 = new kstream<gzFile, FunctorZlib>(in2, gzr);
    }
    this->interleaved = interleaved;
    return 0;
}

int Fxstream::close() {
    if(in1 != NULL) 
    {
      gzclose(in1);
    }
    if(in2 != NULL) {
        gzclose(in2);
    }
    return 0;
}

int Fxstream::read( kseq& read1, kseq& read2) {
    
    int l = ks1->read(read1);
    if(l < 0){
        return 1;
    }
    if(this->interleaved) {
        int k = ks1->read(read2);
        if(k < 0) {
          return 1;
        }
    }
    else if(in2 != NULL) {
        int k = ks2->read(read2);
        if(k < 0) {
          return 1;
        }
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
