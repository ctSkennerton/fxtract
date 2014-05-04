//
//  main.cpp
//  fxtract
//
//  Created by Connor Skennerton on 30/07/13.
//  Copyright (c) 2013 Connor Skennerton. All rights reserved.
//

#include <unistd.h>
#include <cstdio>
#include <cassert>

#include "util.h"
#include "fileManager.h"

#include "Fx.h"
extern "C" {
#include "util/ssearch.h"
#include "util/bpsearch.h"
#include "util/msutil.h"
}
#define VERSION "1.0-alpha"

struct Options
{
    bool   H_flag;
    bool   Q_flag;
    bool   G_flag;
    bool   E_flag;
    bool   x_flag;
    bool   r_flag;
    bool   I_flag;
    char * f_flag;

    Options() : H_flag(false),
                Q_flag(false),
                G_flag(false),
                E_flag(false),
                x_flag(false),
                r_flag(false),
                I_flag(false),
                f_flag(NULL)
    {}
};


void printSingle(Fx * mate1, FILE * out ) {
    mate1->puts(out);
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
    "\t-I           The read file is interleaved (both pairs in a single file)\n"
    "\t-f <file>    File containing patterns, one per line\n"
    "\t-h           Print this help\n"
    "\t-V           Print version\n";
//   exit(1);
//}

int parseOptions(int argc,  char * argv[], Options& opts) {
    int c;
    while ((c = getopt(argc, argv, "HhIf:zjqVrQGEPx")) != -1 ) {
        switch (c) {
            case 'f':
                opts.f_flag = optarg;
                break;

            case 'I':
                opts.I_flag = true;
                break;

            case 'Q':
                opts.Q_flag = true;
                break;
            case 'e':
                opts.r_flag = true;
                break;
            case 'x':
                opts.x_flag = true;
                break;
            case 'V':
                puts(VERSION);
                exit(1);
                break;
            case 'H':
                opts.H_flag = true;
                break;
            case 'r':
                opts.r_flag = true;
                break;
            case 'h':
            default:
                usage(usage_msg);
                break;
        }
    }
    return optind;
}

void split( std::vector<std::string> & theStringVector,  /* Altered/returned value */
       std::string theString,
       const  std::string theDelimiter)
{
    assert(theDelimiter.size() > 0); // My own ASSERT macro.

    size_t  start = 0, end = 0;

    while ( end != std::string::npos)
    {
        end = theString.find( theDelimiter, start);

        // If at end, use length=maxLength.  Else use length=end-start.
        theStringVector.push_back( theString.substr( start,
                       (end == std::string::npos) ? std::string::npos : end - start));

        // If at end, use start=maxSize.  Else use start=end+delimiter.
        start = (   ( end > (std::string::npos - theDelimiter.size()) )
                  ?  std::string::npos  :  end + theDelimiter.size());
    }
}

void tokenizePatternFile(FILE * in, FileManager&  fmanager) {
    // tokenize a line from the pattern file.  The first part will be the pattern and the second
    // part is the file to write to.
    char * lineptr = NULL;
    size_t n;


    while(getline(&lineptr, &n, in) != -1) {
        std::vector<std::string> fields;
        split(fields, lineptr, "\t");
        switch(fields.size()) {
            case 0:
                break;
            case 1:
                fmanager.add(fields[0]);
                break;
            default:
                fmanager.add(fields[0], fields[1]);
                break;
        }
    }
    free(lineptr);
}
static int
on_match(int strnum, const char *textp, void const * context)
{
	Fx * read = (Fx *) context;
    read->puts(stdout);
    return  1;
}

int main(int argc, char * argv[])
{
    //kvec_t(sds) pattern_list;
    FileManager manager;
    Options opts;

    int opt_idx = parseOptions(argc, argv, opts);

    if(opts.f_flag == NULL) {

        if( opt_idx >= argc) {
            puts("Please provide a pattern (or pattern file) and at least one input file");
            usage(usage_msg);
        } else if (opt_idx >= argc - 1) {
            puts("Please provide an input file (or two)");
            usage(usage_msg);
        }
        std::string pattern = argv[opt_idx];
        ++opt_idx;
        manager.add(pattern);
        if(opts.r_flag) {
          std::string rcpattern = pattern;
            reverseComplement(rcpattern);
            manager.add(rcpattern);
        }


    } else {
        if (opt_idx > argc - 1) {
            puts("Please provide an input file (or two)");
            usage(usage_msg);
        }
        FILE * in = fopen(opts.f_flag, "r");
        tokenizePatternFile(in, manager);
    }

    Fxstream stream;
    if(opt_idx == argc - 2) {
        // two read files
        stream.open(argv[opt_idx], argv[opt_idx+1], opts.I_flag);
    } else if (opt_idx == argc - 1) {
        // one read file
        stream.open(argv[opt_idx], NULL, opts.I_flag);
    }


    Fx * mate1 = new Fx();
    Fx * mate2 = new Fx();


    std::string conc;
    std::map<std::string, int>::iterator iter;
    for (iter = manager.patternMapping.begin(); iter != manager.patternMapping.end() ; ++iter ) {
        conc += iter->first + "\n";
    }
    char * concstr  = (char *) malloc(conc.size() * sizeof(char *));
    strncpy(concstr, conc.c_str(), conc.size());
    concstr[conc.size()-1] = '\0';
    int npatts;
    MEMREF *pattv =  refsplit(concstr, '\n', &npatts);

    SSEARCH *ssp = ssearch_create(pattv, npatts);

    while(stream.read(&mate1, &mate2) >= 0) {
        MEMREF data = {mate1->seq, strlen(mate1->seq)};
        int ret = ssearch_scan(ssp, data, (SSEARCH_CB)on_match, (void *)mate1);
        if(ret == 0){
            // read one did not have a match check read 2 if it exists
            if(mate2 != NULL) {
                MEMREF data2 = {mate2->seq, strlen(mate2->seq)};
                ssearch_scan(ssp, data2, (SSEARCH_CB)on_match, (void *)mate2);
            }
        }
    }

    free(concstr);
    delete mate1;
    delete mate2;
    stream.close();
    free(pattv);

    return 0;
}
