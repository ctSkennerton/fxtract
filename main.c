//
//  main.cpp
//  fxtract
//
//  Created by Connor Skennerton on 30/07/13.
//  Copyright (c) 2013 Connor Skennerton. All rights reserved.
//

#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>

#include "util.h"
#include "fileManager.h"
#include "kseq.h"
#include "khash.h"
#include "sds/sds.h"
#include "Fx.h"
#include "aho-corasick/msutil.h"
#include "aho-corasick/acism.h"

#define VERSION "1.0-alpha"

typedef struct _Options
{
    bool   gzip;
    bool   bzip2;
    bool   fasta;
    bool   fastq;
    bool   H_flag;
    bool   Q_flag;
    bool   G_flag;
    bool   x_flag;
    bool   r_flag;
    bool   I_flag;
    char * f_flag;
} Options;

KHASH_INIT(s2s, sds, sds, 1, kh_str_hash_func, kh_str_hash_equal);


void options_init(Options * opts) {
    opts->gzip = false;
    opts->bzip2 = false;
    opts->fasta = false;
    opts->fastq = false;
    opts->H_flag = false;
    opts->f_flag = NULL;
    opts->r_flag =false;
    opts->I_flag = false;
}


void printSingle(Fx * mate1, FILE * out ) {
    if (mate1->qualStart) {
        fputc('@', out);
        fwrite(mate1->data + mate1->headerStart, 1, mate1->headerLen, out);
        fputc('\n', out);
        fwrite(mate1->data + mate1->seqStart, 1, mate1->seqLen, out);
        fputs("\n+\n", out);
        fwrite(mate1->data + mate1->qualStart, 1, mate1->qualLen, out);
        fputc('\n', out);
    } else {
        fputc('>', out);
        fwrite(mate1->data + mate1->headerStart, 1, mate1->headerLen, out);
        fputc('\n', out);
        fwrite(mate1->data + mate1->seqStart, 1, mate1->seqLen, out);
        fputc('\n', out);
    }
}

void printPair(Fx* mate1, Fx* mate2, FILE * out) {
    // match in the first read print out pair
    printSingle(mate1, out);
    printSingle(mate2, out);
}

//void usage() {
static const char usage_msg[] =\
    "[-hHv] {-f pattern_file | pattern} <read1.fx> [<read2.fx>]\n"
    "\t-H           Evaluate patterns in the context of headers (default: sequences)\n"
    "\t-Q           Evaluate patterns in the context of quality scores (default: sequences)\n"
    "\t-G           pattern is a posix basic regular expression (default: literal substring)\n"
    "\t-E           pattern is a posix extended regular expression (default: literal substring)\n"
    "\t-P           pattern is a perl compatable regular expression (default: literal substring)\n"
    "\t-x           pattern exactly matches the whole string (default: literal substring)\n"
    "\t-I           The read file in interleaved (both pairs in a single file)\n"
    //puts("\t-j           Force bzip2 formatting");
    //puts("\t-q           Force fastq formatting");
    "\t-f <file>    File containing patterns, one per line\n"
    "\t-h           Print this help\n"
    "\t-V           Print version\n";
//   exit(1);
//}

int parseOptions(int argc,  char * argv[], Options* opts) {
    int c;
    while ((c = getopt(argc, argv, "HhIf:zjqVrQGEPx")) != -1 ) {
        switch (c) {
            case 'f':
                opts->f_flag = optarg;
                break;
            case 'z':
                opts->gzip = true;
                break;
            case 'I':
                opts->I_flag = true;
                break;
            case 'j':
                opts->bzip2 = true;
                break;
            case 'Q':
                opts->Q_flag = true;
                break;
            case 'e':
                opts->r_flag = true;
                break;
            case 'x':
                opts->x_flag = true;
                break;
            case 'V':
                puts(VERSION);
                exit(1);
                break;
            case 'H':
                opts->H_flag = true;
                break;
            case 'r':
                opts->r_flag = true;
                break;
            case 'h':
            default:
                usage(usage_msg);
                break;
        }
    }
    return optind;
}

void tokenizePatternFile(FILE * in, FileManager * fmanager) {
    // tokenize a line from the pattern file.  The first part will be the pattern and the second
    // part is the file to write to.
    char * lineptr = NULL;
    size_t n;


    while(getline(&lineptr, &n, in) != -1) {
        int fields;
        sds* split_line = sdssplitlen(lineptr, n, "\t", 1, &fields);
        switch(fields) {
            case 0:
                break;
            case 1:
                filemanager_add(fmanager, split_line[0]);
                break;
            default:
                filemanager_add2(fmanager, split_line[0], split_line[1]);
                break;
        }
        sdsfreesplitres(split_line, fields);
    }
    free(lineptr);
}

static int
on_match(int strnum, int textpos, MEMREF const *pattv)
{
    (void)strnum, (void)textpos, (void)pattv;
    //++actual;
    fprintf(stdout, "%9d %7d '%.*s'\n", textpos, strnum, (int)pattv[strnum].len, pattv[strnum].ptr);
    return 0;
}

int main(int argc, char * argv[])
{
    kvec_t(sds) pattern_list;
    FileManager * manager = filemanager_new();
    Options opts;
    options_init(&opts);

    int opt_idx = parseOptions(argc, argv, &opts);

    if(opts.f_flag == NULL) {

        if( opt_idx >= argc) {
            puts("Please provide a pattern (or pattern file) and at least one input file");
            usage(usage_msg);
        } else if (opt_idx >= argc - 1) {
            puts("Please provide an input file (or two)");
            usage(usage_msg);
        }
        sds pattern = sdsnew(argv[opt_idx++]);
        filemanager_add(manager, pattern);
        if(opts.r_flag) {
            sds rcpattern = sdsdup(pattern);
            reverseComplement(rcpattern, sdslen(rcpattern));
            filemanager_add(manager, rcpattern);
        }


    } else {
        if (opt_idx > argc - 1) {
            puts("Please provide an input file (or two)");
            usage(usage_msg);
        }
        FILE * in = fopen(opts.f_flag, "r");
        tokenizePatternFile(in, manager);
    }

    Fxstream * stream = NULL;
    if(opt_idx == argc - 2) {
        // two read files
        stream = fxstream_open(argv[opt_idx+1],argv[opt_idx+2],opts.I_flag);
    } else if (opt_idx == argc -1) {
        // one read file
        stream = fxstream_open(argv[opt_idx+1], NULL, opts.I_flag);
    }

    if(NULL == stream) {
        fprintf(stderr, "problem opening input files\n");
        exit(2);
    }

    Fx * mate1 = fx_new();
    Fx * mate2 = fx_new();

    MEMBUF patt = chomp(slurp(argv[1]));
    if (!patt.ptr)
        die("cannot read %s", argv[1]);

    int npatts = filemanager_npat(manager);
    MEMREF *pattv =  malloc(npatts * sizeof(MEMREF));  //refsplit(patt.ptr, '\n', &npatts);
    int i;
    for (khiter_t k = kh_begin(manager->patternMapping), i = 0; k != kh_end(manager->patternMapping); ++k, ++i) {
        pattv[i].ptr = kh_key(manager->patternMapping, k);
        pattv[i].len = sdslen(kh_key(manager->patternMapping, k));
    }

    ACISM *psp = acism_create(pattv, npatts);
    while(fxstream_read(stream, mate1, mate2) >= 0) {
        int state;
        MEMREF data = {mate1->data, sdslen(mate1->data)};
        (void)acism_more(psp, data, (ACISM_ACTION*)on_match, pattv, &state);
        //sds repr = sdsempty();
        //fx_repr(read, &repr);
        //puts(repr);
        //sdsfree(repr);
    }
    fx_delete(mate1);
    fx_delete(mate2);
    fxstream_close(stream);
    free(pattv);
    filemanager_delete(manager);

    return 0;
}

