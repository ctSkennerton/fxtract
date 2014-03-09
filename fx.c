#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include "fx.h"
KSEQ_INIT(int, read);

struct _Fxstream {
    kseq_t * seq1; // array of input files one for each mate in the pair
    kseq_t * seq2; // array of input files one for each mate in the pair
    int      fd1;
    int      fd2;
    bool     interleaved;
};

Fx * fx_new() {
    Fx * fx = malloc(sizeof(Fx));
    fx->data = sdsempty();
    return fx;
}

Fx * fx_new2(char * d) {
    Fx * fx = malloc(sizeof(Fx));
    fx->data = sdsnew(d);
    return fx;
}

void fx_delete(Fx * fx) {
    sdsfree(fx->data);
    free(fx);
}

void fx_repr(Fx * fx, sds * reprString) {
    (*reprString) = sdscpy((*reprString), fx->data);
    (*reprString) = sdscatprintf((*reprString), "\n%*c%*c", fx->headerLen, '|', fx->seqLen, '|');
    if(fx->qualStart != -1) {
        (*reprString) = sdscatprintf((*reprString), "%*c", fx->qualLen, '|');
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

int fxstream_read(Fxstream * stream, Fx * read1, Fx * read2) {
    int len1, len2;
    len1 = kseq_read(stream->seq1);
    if(len1 < 0) {
        read1 = NULL;
        read2 = NULL;
        return len1;
    }
    
    read1->data = sdscpy(read1->data, stream->seq1->name.s);
    read1->headerStart = 0;
    read1->headerLen = strlen(stream->seq1->name.s);
    read1->data = sdscat(read1->data, stream->seq1->seq.s);
    read1->seqStart = read1->headerLen;
    read1->seqLen = strlen(stream->seq1->seq.s);

    if(stream->seq1->qual.s) {
        read1->data = sdscat(read1->data, stream->seq1->qual.s);
        read1->qualStart = read1->headerLen + read1->seqLen;
        read1->qualLen = strlen(stream->seq1->qual.s);
    } else {
        read1->qualStart = -1;
        read1->qualLen = -1;
    }

    if(stream->interleaved) {
        len2 = kseq_read(stream->seq1);
        if(len2 < 0) {
            read1 = NULL;
            read2 = NULL;
            return len2;
        }
        read2->data = sdscpy(read2->data, stream->seq1->name.s);
        read2->headerStart = 0;
        read2->headerLen = strlen(stream->seq1->name.s);
        read2->data = sdscat(read2->data, stream->seq1->seq.s);
        read2->seqStart = read2->headerLen;
        read2->seqLen = strlen(stream->seq1->seq.s);

        if(stream->seq1->qual.s) {
            read2->data = sdscat(read2->data, stream->seq1->qual.s);
            read2->qualStart = read2->headerLen + read2->seqLen;
            read2->qualLen = strlen(stream->seq1->qual.s);
        } else {
            read2->qualStart = -1;
            read2->qualLen = -1;
        }
    } else if(stream->seq2 != NULL) {
        len2 = kseq_read(stream->seq2);
        if(len2 < 0) {
            read1 = NULL;
            read2 = NULL;
            return len2;
        }
        read2->data = sdscpy(read2->data, stream->seq1->name.s);
        read2->headerStart = 0;
        read2->headerLen = strlen(stream->seq1->name.s);
        read2->data = sdscat(read2->data, stream->seq1->seq.s);
        read2->seqStart = read2->headerLen;
        read2->seqLen = strlen(stream->seq1->seq.s);

        if(stream->seq1->qual.s) {
            read2->data = sdscat(read2->data, stream->seq1->qual.s);
            read2->qualStart = read2->headerLen + read2->seqLen;
            read2->qualLen = strlen(stream->seq1->qual.s);
        } else {
            read2->qualStart = -1;
            read2->qualLen = -1;
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
    if(fxstream_read(stream, read, NULL) >= 0) {
        sds repr = sdsempty();
        fx_repr(read, &repr);
        puts(repr);
        sdsfree(repr);
    }
    fx_delete(read);
    fxstream_close(stream);
    return 0;
}
#endif
