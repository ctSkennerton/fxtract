#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstdio>
#include "fx.h"


int Fx::puts(FILE * out) {
    if(qual == NULL) {
        return fprintf(out, ">%s\n%s\n", name, seq);
    } else {
        return fprintf(out, "@%s\n%s\n+\n%s\n", name, seq, qual);
    }
}

int Fxstream::open(const char * file1, const char * file2, bool interleaved) {
    int fd1 = -1;
    int fd2 = -1;
    fd1 = ::open(file1, O_RDONLY);
    if(fd1 == -1) {
        perror("failed to open file for mate1");
        return 1;
    }
    if(file2 != NULL && interleaved) {
        fprintf(stderr, "When the interleaved flag is set file2 must equal NULL");
        return 1;
    } else if (file2 != NULL && !interleaved) {
        fd2 = ::open(file2, O_RDONLY);
        if(fd2 == -1) {
            perror("failed to open file for mate2");
            return 1;
        }
    }
    this->interleaved = interleaved;
    this->fd1 = fd1;
    this->fd2 = fd2;
    this->seq1 = kseq_init(fd1);
    if(-1 != fd2) {
        this->seq2 = kseq_init(fd2);
    } else {
        this->seq2 = NULL;
    }
    return 0;
}

int Fxstream::close() {
    kseq_destroy(this->seq1);
    ::close(this->fd1);
    if(this->seq2) {
        kseq_destroy(this->seq2);
        ::close(this->fd2);
    }
    return 0;
}

//TODO: BSD grep has support for zip, bzip2 and xz files out of the box
//http://svnweb.freebsd.org/base/head/usr.bin/grep/file.c?view=markup
int Fxstream::read( Fx ** read1, Fx ** read2) {
    int len1, len2;
    len1 = kseq_read(this->seq1);
    if(len1 < 0) {
        (*read1) = NULL;
        (*read2) = NULL;
        return len1;
    }
    (*read1)->name = (char *) malloc(sizeof(char *) * this->seq1->name.l);
    (*read1)->name = strncpy((*read1)->name, this->seq1->name.s, this->seq1->name.l);

    (*read1)->seq = (char *) malloc(sizeof(char *) * this->seq1->seq.l);
    (*read1)->seq = strncpy((*read1)->seq, this->seq1->seq.s, this->seq1->seq.l);

    if(this->seq1->qual.s) {
        (*read1)->qual = (char *) malloc(sizeof(char *) * this->seq1->qual.l);
        (*read1)->qual = strncpy((*read1)->qual, this->seq1->qual.s, this->seq1->qual.l);
    }

    if(this->interleaved) {
        len2 = kseq_read(this->seq1);
        if(len2 < 0) {
            (*read1) = NULL;
            (*read2) = NULL;
            return len2;
        }
        (*read2)->name = (char *) malloc(sizeof(char *) * this->seq1->name.l);
        (*read2)->name = strncpy((*read2)->name, this->seq1->name.s, this->seq1->name.l);

        (*read2)->seq = (char *) malloc(sizeof(char *) * this->seq1->seq.l);
        (*read2)->seq = strncpy((*read2)->seq, this->seq1->seq.s, this->seq1->seq.l);

        if(this->seq1->qual.s) {
          (*read2)->qual = (char *) malloc(sizeof(char *) * this->seq1->qual.l);
          (*read2)->qual = strncpy((*read2)->qual, this->seq1->qual.s, this->seq1->qual.l);
        }

    } else if(this->seq2 != NULL) {
        len2 = kseq_read(this->seq2);
        if(len2 < 0) {
            (*read1) = NULL;
            (*read2) = NULL;
            return len2;
        }
        (*read2)->name = (char *) malloc(sizeof(char *) * this->seq1->name.l);
        (*read2)->name = strncpy((*read2)->name, this->seq1->name.s, this->seq1->name.l);

        (*read2)->seq = (char *) malloc(sizeof(char *) * this->seq1->seq.l);
        (*read2)->seq = strncpy((*read2)->seq, this->seq1->seq.s, this->seq1->seq.l);

        if(this->seq1->qual.s) {
          (*read2)->qual = (char *) malloc(sizeof(char *) * this->seq1->qual.l);
          (*read2)->qual = strncpy((*read2)->qual, this->seq1->qual.s, this->seq1->qual.l);
        }
    } else {
        (*read2) = NULL;
    }
    return len1;
}



#ifdef TEST_FXREAD_MAIN
int main(int argc, char * argv[]) {
    Fxstream stream;
    stream.open(argv[1], NULL, false);
    Fx * read = new Fx();
    while(stream.read(read, NULL) >= 0) {
        printf(">%s\n%s\n", read->name, read->seq);
        read->puts(stdout);
    }
    delete read;
    stream.close();
    return 0;
}
#endif
