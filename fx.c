#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include "fx.h"
KSEQ_INIT(int, read);

struct _Fxstream {
    kseq_t * seq1;
    kseq_t * seq2;
    int      fd1;
    int      fd2;
    bool     interleaved;
};

Fx * fx_new() {
    Fx * fx  = malloc(sizeof(Fx));
    fx->name = sdsempty();
    fx->seq  = sdsempty();
    fx->qual = sdsempty();
    return fx;
}

void fx_delete(Fx * fx) {
    sdsfree(fx->name);
    sdsfree(fx->seq);
    sdsfree(fx->qual);
    free(fx);
}

int fx_fputs(Fx * fx, FILE * out) {
    if(fx->qual != NULL) {
        return fprintf(out, ">%s\n%s\n", fx->name, fx->seq);
    } else {
        return fprintf(out, "@%s\n%s\n+\n%s\n", fx->name, fx->seq, fx->qual);
    }
}

Fxstream * fxstream_open(const char * file1, const char * file2, bool interleaved) {
    int fd1 = -1;
    int fd2 = -1;
    fd1 = open(file1, O_RDONLY);
    if(fd1 == -1) {
        perror("failed to open file for mate1");
        return NULL;
    }
    if(file2 != NULL && interleaved) {
        fprintf(stderr, "When the interleaved flag is set file2 must equal NULL");
        return NULL;
    } else if (file2 != NULL && !interleaved) {
        fd2 = open(file2, O_RDONLY);
        if(fd2 == -1) {
            perror("failed to open file for mate2");
            return NULL;
        }
    }
    Fxstream * fs = (Fxstream *) malloc(sizeof(Fxstream));
    fs->interleaved = interleaved;
    fs->fd1 = fd1;
    fs->fd2 = fd2;
    fs->seq1 = kseq_init(fd1);
    if(-1 != fd2) {
        fs->seq2 = kseq_init(fd2);
    } else {
        fs->seq2 = NULL;
    }
    return fs;
}

//TODO: BSD grep has support for zip, bzip2 and xz files out of the box
//http://svnweb.freebsd.org/base/head/usr.bin/grep/file.c?view=markup
int fxstream_read(Fxstream * stream, Fx * read1, Fx * read2) {
    int len1, len2;
    len1 = kseq_read(stream->seq1);
    if(len1 < 0) {
        read1 = NULL;
        read2 = NULL;
        return len1;
    }

    read1->name = sdscpylen(read1->name, stream->seq1->name.s, stream->seq1->name.l);
    read1->seq = sdscpylen(read1->seq, stream->seq1->seq.s, stream->seq1->seq.l);

    if(stream->seq1->qual.s) {
        read1->qual = sdscpylen(read1->qual, stream->seq1->qual.s, stream->seq1->qual.l);
    }

    if(stream->interleaved) {
        len2 = kseq_read(stream->seq1);
        if(len2 < 0) {
            read1 = NULL;
            read2 = NULL;
            return len2;
        }
        read2->name = sdscpylen(read2->name, stream->seq1->name.s, stream->seq1->name.l);
        read2->seq = sdscpylen(read2->seq, stream->seq1->seq.s, stream->seq1->seq.l);

        if(stream->seq1->qual.s) {
            read2->qual = sdscpylen(read2->qual, stream->seq1->qual.s, stream->seq1->qual.l);
        }

    } else if(stream->seq2 != NULL) {
        len2 = kseq_read(stream->seq2);
        if(len2 < 0) {
            read1 = NULL;
            read2 = NULL;
            return len2;
        }
        read2->name = sdscpylen(read2->name, stream->seq2->name.s, stream->seq2->name.l);
        read2->seq = sdscpylen(read2->seq, stream->seq2->seq.s, stream->seq2->seq.l);

        if(stream->seq2->qual.s) {
            read2->qual = sdscpylen(read2->qual, stream->seq2->qual.s, stream->seq2->qual.l);
        }
    }
    return len1;
}

void fxstream_close(Fxstream * stream) {
    kseq_destroy(stream->seq1);
    close(stream->fd1);
    if(stream->seq2) {
        kseq_destroy(stream->seq2);
        close(stream->fd2);
    }
    free(stream);
}

#ifdef TEST_FXREAD_MAIN
int main(int argc, char * argv[]) {
    Fxstream * stream = fxstream_open(argv[1], NULL, false);
    Fx * read = fx_new();
    while(fxstream_read(stream, read, NULL) >= 0) {
        printf(">%s\n%s\n", read->name, read->seq);
    }
    fx_delete(read);
    fxstream_close(stream);
    return 0;
}
#endif
