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
#include <regex.h>
#ifdef HAVE_PCRE
#include <pcre.h>
#endif

#include <iostream>
#include <fstream>
#include <string>

#include "util.h"
#include "fileManager.h"
#include "fx.h"

extern "C" {
#include "util/ssearch.h"
#include "util/bpsearch.h"
#include "util/msutil.h"
}
#define VERSION "1.0-alpha"

#ifndef REG_BASIC
#define REG_BASIC 0
#endif
struct Options
{
    bool   H_flag;
    bool   Q_flag;
    bool   G_flag;
    bool   E_flag;
    bool   X_flag;
    bool   r_flag;
    bool   I_flag;
#ifdef HAVE_PCRE
    bool   P_flag;
#endif
    char * f_flag;

    Options() : H_flag(false),
                Q_flag(false),
                G_flag(false),
                E_flag(false),
                X_flag(false),
                r_flag(false),
                I_flag(false),
#ifdef HAVE_PCRE
                P_flag(false),
#endif
                f_flag(NULL)
    {}
};


void printSingle(Fx ** mate, FILE * out ) {
    if(mate != NULL) {
        fprintf(out, ">%s %s\n%s\n", (*mate)->name.c_str(), (*mate)->comment.c_str(), (*mate)->seq.c_str());
        //mate->puts(out);
    }
}

void printPair(Fx ** mate1, Fx ** mate2, FILE * out) {
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
#ifdef HAVE_PCRE
    "\t-P           pattern is a perl compatable regular expression (default: literal substring)\n"
#endif
    "\t-X           pattern exactly matches the whole string (default: literal substring)\n"
    "\t-I           The read file is interleaved (both pairs in a single file)\n"
    "\t-f <file>    File containing patterns, one per line\n"
    "\t-h           Print this help\n"
    "\t-V           Print version\n";
//   exit(1);
//}

int parseOptions(int argc,  char * argv[], Options& opts) {
    int c;
    while ((c = getopt(argc, argv, "HhIf:zjqVrQGEPX")) != -1 ) {
        switch (c) {
            case 'f':
                opts.f_flag = optarg;
                break;
            case 'E':
                opts.E_flag = true;
                break;
            case 'G':
                opts.G_flag = true;
                break;
            case 'I':
                opts.I_flag = true;
                break;
#ifdef HAVE_PCRE
            case 'P':
                opts.P_flag = true;
                break;
#endif
            case 'Q':
                opts.Q_flag = true;
                break;
            /*case 'e':
                opts.e_flag = true;
                break;*/
            case 'X':
                opts.X_flag = true;
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

void tokenizePatternFile(std::ifstream& in, FileManager&  fmanager) {
    // tokenize a line from the pattern file.  The first part will be the pattern and the second
    // part is the file to write to.

    std::string lineptr;

    while(in >> lineptr) {
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
}
static int
on_match(int strnum, const char *textp, void const * context)
{
    return  1;
}

int hash_search(FileManager& manager, Fxstream& stream, Options& opts) {


    Fx mate1, mate2;
    std::map<std::string, int>::iterator iter;
    while(stream.read(mate1, mate2) == 0) {
        std::string * data;
        if(opts.H_flag) {
            data = &mate1.name;

        } else if(opts.Q_flag){
            if(!mate1.isFasta()) {
                data = &mate1.qual;
            }
        } else {
            data = &mate1.seq;
        }
        FILE * out = manager.find(*data);
        if(out != NULL) {
            std::cout << mate1 << mate2;

        } else {
            // read one did not have a match check read 2 if it exists
            if(!mate2.empty()) {
                if(opts.H_flag) {
                    data = &mate2.name;

                } else if(opts.Q_flag){
                    if(!mate2.isFasta()) {
                        data = &mate2.seq;
                    }
                } else {
                    data = &mate2.seq;
                }
            }
            FILE * out = manager.find(*data);
            if(out != NULL) {
                std::cout << mate1 << mate2;
            }
        }
        mate1.clear();
        mate2.clear();
    }

    return 0;
}

int multipattern_search(FileManager& manager, Fxstream& stream, Options& opts) {


    Fx mate1, mate2;

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

    if(ssp == NULL) {
        fprintf(stderr, "problem initalizing multi-search");
        return 1;
    }

    while(stream.read(mate1, mate2) == 0) {
        MEMREF data;
        if(opts.H_flag) {
            data.ptr = mate1.name.c_str();
            data.len = mate1.name.size();

        } else if(opts.Q_flag){
            if(!mate1.isFasta()) {
                data.ptr = mate1.qual.c_str();
                data.len = (size_t)mate1.len;
            }
        } else {
            data.ptr = mate1.seq.c_str();
            data.len = (size_t)mate1.len;
        }
        int ret = ssearch_scan(ssp, data, (SSEARCH_CB)on_match, NULL);
        if(ret) {
            FILE * out = manager.find(data.ptr);
            if(out != NULL) {
                std::cout << mate1 << mate2;
            }
        } else {
            // read one did not have a match check read 2 if it exists
            if(!mate2.empty()) {
                if(opts.H_flag) {
                    data.ptr = mate2.name.c_str();
                    data.len = mate2.name.size();

                } else if(opts.Q_flag){
                    if(!mate2.isFasta()) {
                        data.ptr = mate2.seq.c_str();
                        data.len = (size_t)mate2.len;
                    }
                } else {
                    data.ptr = mate2.seq.c_str();
                    data.len = (size_t)mate2.len;
                }
                ret = ssearch_scan(ssp, data, (SSEARCH_CB)on_match, NULL);
                if(ret) {
                    FILE * out = manager.find(data.ptr);
                    if(out != NULL) {
                        std::cout << mate1 << mate2;
                    }
                }
            }
        }
        mate1.clear();
        mate2.clear();
    }

    free(concstr);
    free(pattv);
    ssearch_destroy(ssp);
    return 0;
}


char *get_regerror (int errcode, regex_t *compiled) {
    size_t length = regerror (errcode, compiled, NULL, 0);
    char *buffer = (char *)malloc (length);
    (void) regerror (errcode, compiled, buffer, length);
    return buffer;
}

int posix_regex_search(FileManager& manager, Fxstream& stream, Options& opts, regex_t * pxr) {


    Fx mate1, mate2;

    while(stream.read(mate1, mate2) == 0) {
        std::string * data = &mate1.seq;
        if(opts.H_flag) {
            data = &mate1.name;

        } else if(opts.Q_flag){
            if(!mate1.isFasta()) {
                data = &mate1.qual;
            }
        }

        int ret = regexec(pxr, data->c_str(), 0, NULL, 0);
        if(ret == REG_NOMATCH){
            // read one did not have a match check read 2 if it exists
            if(!mate2.empty()) {
                if(opts.H_flag) {
                    data = &mate2.name;

                } else if(opts.Q_flag){
                    if(!mate2.isFasta()) {
                        data = &mate2.qual;
                    }
                } else {
                    data = &mate2.seq;
                }
                ret = regexec(pxr, data->c_str(), 0, NULL, 0);
                if(ret != (REG_NOMATCH | 0)) {
                    char * errorbuf = get_regerror(ret, pxr);
                    fprintf(stderr, "%s\n", errorbuf);
                    free(errorbuf);
                    return 1;
                } else if(ret == 0) {
                    std::cout << mate1 << mate2;

                }
            }
        } else if(ret != 0) {
            char * errorbuf = get_regerror(ret, pxr);
            fprintf(stderr, "%s\n", errorbuf);
            free(errorbuf);
            return 1;
        } else {
            std::cout << mate1 << mate2;

        }
        mate1.clear();
        mate2.clear();
    }

    return 0;
}
#ifdef HAVE_PCRE
int pcre_search(FileManager& manager, Fxstream& stream, Options& opts, pcre * pxr) {
    Fx mate1, mate2;
    int ovector[30];

    while(stream.read(mate1, mate2) == 0) {
        std::string * data = &mate1.seq;
        if(opts.H_flag) {
            data = &mate1.name;

        } else if(opts.Q_flag){
            if(!mate1.isFasta()) {
                data = &mate1.qual;
            }
        } 
        int ret = pcre_exec(pxr, NULL, data->c_str(), data->size(), 0, 0, ovector, 30);
        if(ret != 1){
            // read one did not have a match check read 2 if it exists
            if(!mate2.empty()) {
                if(opts.H_flag) {
                    data = &mate2.name;

                } else if(opts.Q_flag){
                    if(!mate2.isFasta()) {
                        data = &mate2.seq;
                    }
                } else {
                    data = &mate2.seq;
                }
                ret = pcre_exec(pxr, NULL, data->c_str(), data->size(), 0, 0, ovector, 30);
                if(ret == 1) {
                    std::cout << mate1 << mate2;
                }
            }
        } else {
            std::cout << mate1 << mate2;
        }
        mate1.clear();
        mate2.clear();
    }
    return 0;
}
#endif

int simple_string_search(FileManager& manager, Fxstream& stream, Options& opts, const char * pattern) {
    Fx mate1;
    Fx mate2;

    while(stream.read(mate1, mate2) == 0) {
        std::string * data = &mate1.seq;
        if(opts.H_flag) {
            data = &mate1.name;

        } else if(opts.Q_flag){
            if(!mate1.isFasta()) {
                data = &mate1.qual;
            }
        }

        const char * ret = strstr(data->c_str(), pattern);
        if(ret == NULL){
            // read one did not have a match check read 2 if it exists
            if(!mate2.empty()) {
                if(opts.H_flag) {
                    data = &mate2.name;

                } else if(opts.Q_flag){
                    if(!mate2.isFasta()) {
                        data = &mate2.qual;
                    }
                } else {
                    data = &mate2.seq;
                }
                ret = strstr(data->c_str(), pattern);
                if(ret != NULL) {
                    std::cout << mate1 << mate2;

                }
            }
        } else {
            std::cout << mate1 << mate2;
        }
        mate1.clear();
        mate2.clear();
    }
    return 0;

}

int main(int argc, char * argv[])
{
    FileManager manager;
    Options opts;

    int opt_idx = parseOptions(argc, argv, opts);
    std::string pattern;
    if(opts.f_flag == NULL) {

        if( opt_idx >= argc) {
            puts("Please provide a pattern (or pattern file) and at least one input file");
            usage(usage_msg);
        } else if (opt_idx >= argc - 1) {
            puts("Please provide an input file (or two)");
            usage(usage_msg);
        }
        pattern = argv[opt_idx];
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
        std::ifstream in (opts.f_flag);
        if(!in.good()) {
            fprintf(stderr, "problem opening pattern file\n");
            exit(1);
        } else {
            tokenizePatternFile(in, manager);
        }
    }

    Fxstream stream;
    if(opt_idx == argc - 2) {
        // two read files
        stream.open(argv[opt_idx], argv[opt_idx+1], opts.I_flag);
    } else if (opt_idx == argc - 1) {
        // one read file
        stream.open(argv[opt_idx], NULL, opts.I_flag);
    }

    if(opts.E_flag | opts.G_flag) {
        // Posix  regex
        regex_t px_regex;
        int flags = REG_NOSUB;  //only need to report success or failure
        if(opts.E_flag) {
            flags |= REG_EXTENDED;
        } else {
            flags |= REG_BASIC;
        }
        int ret = regcomp(&px_regex, pattern.c_str(), flags);
        if(ret) {
            char * errorbuf = get_regerror(ret, &px_regex);
            fprintf(stderr, "%s\n", errorbuf);
            free(errorbuf);
            stream.close();
            return 1;
        }
        ret = posix_regex_search(manager, stream, opts, &px_regex);
        regfree(&px_regex);
        stream.close();
        return ret;
    }
#ifdef HAVE_PCRE
    else if (opts.P_flag) {

        // PCRE regex
        pcre *re;
        const char *error;
        int erroffset;

        re = pcre_compile(
            pattern.c_str(),              /* the pattern */
            0,                    /* default options */
            &error,               /* for error message */
            &erroffset,           /* for error offset */
            NULL);                /* use default character tables */
        if (re == NULL) {
            printf("PCRE compilation failed at offset %d: %s\n", erroffset, error);
            return 1;
        }

        pcre_search(manager, stream, opts, re);
        pcre_free(re);
        stream.close();

    }
#endif
    else if(opts.X_flag) {
        hash_search(manager, stream, opts);
    }
    else if(opts.f_flag) {
        multipattern_search(manager, stream, opts);

    } else {
        simple_string_search(manager, stream, opts, pattern.c_str());
    }

    stream.close();
    return 0;
}
