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
#include <cstring>

#include "util.h"
#include "fileManager.h"
#include "fx.h"
#include "version.h"

extern "C" {
#include "util/ssearch.h"
#include "util/msutil.h"
}

// BSD defines this but linux does not
#ifndef REG_BASIC
#define REG_BASIC 0
#endif
struct Options
{
    bool   H_flag;
    bool   Q_flag;
    bool   C_flag;
    bool   G_flag;
    bool   E_flag;
    bool   X_flag;
    bool   r_flag;
    bool   I_flag;
    bool   P_flag;
    char * f_flag;
    bool   v_flag;
    bool   one_flag;
    bool   two_flag;
    bool   S_flag;
    bool   c_flag;
    bool   l_flag;
    bool   L_flag;

    Options() : H_flag(false),
                Q_flag(false),
                C_flag(false),
                G_flag(false),
                E_flag(false),
                X_flag(false),
                r_flag(false),
                I_flag(false),
                P_flag(false),
                f_flag(NULL),
                v_flag(false),
                one_flag(false),
                two_flag(false),
                S_flag(false),
                c_flag(false),
                l_flag(false),
                L_flag(false)
    {}
};

Options opts;

FileManager manager;

static int matched_records = 0;
MEMREF *pattv;

static const char usage_msg[] =\
    "[options] {-f pattern_file | pattern} {<read1.fx> <read2.fx>}...\n"
    "       fxtract [options] -I {-f pattern_file | pattern} <read12.fx>...\n"
    "       fxtract [options] -S {-f pattern_file | pattern} <read.fx>...\n"
    "\t-H           Evaluate patterns in the context of headers (default: sequences)\n"
    "\t-Q           Evaluate patterns in the context of quality scores (default: sequences)\n"
    "\t-C           Evaluate patters in the context of comment strings - everything after\n"
    "\t             the first space on the header line of the record (default: sequences)\n"
    "\t-G           pattern is a posix basic regular expression (default: literal substring)\n"
    "\t-E           pattern is a posix extended regular expression (default: literal substring)\n"
#ifdef HAVE_PCRE
    "\t-P           pattern is a perl compatable regular expression (default: literal substring)\n"
#endif
    "\t-X           pattern exactly matches the whole string (default: literal substring)\n"
    "\t-r           Match the reverse complement of a literal pattern. Not compatible with regular expressions\n"
    "\t-I           The read file is interleaved (both pairs in a single file)\n"
    "\t-S           The files do not contain pairs. Allows for multiple files to be given on the command line\n"
    //"\t-1           Print only read 1 whether there was a match in read 1 or read 2\n"
    //"\t-2           Print only read 2 whether there was a match in read 1 or read 2\n"
    "\t-v           Inverse the match criteria. Print pairs that do not contain matches\n"
    "\t-c           Print only the count of reads (or pairs) that were found\n"
    "\t-f <file>    File containing patterns, one per line\n"
    "\t-l           Print file names that contain matches\n"
    //"\t-L           Print file names that contain no matches\n"
    "\t-h           Print this help\n"
    "\t-V           Print version\n";

int parseOptions(int argc,  char * argv[]) {
    int c;
    while ((c = getopt(argc, argv, "HhCcIf:zjqVvrQGEPX12SlL")) != -1 ) {
        switch (c) {
            case 'c':
                opts.c_flag = true;
                break;
            case 'E':
                opts.E_flag = true;
                break;
            case 'f':
                opts.f_flag = optarg;
                break;
            case 'G':
                opts.G_flag = true;
                break;
            case 'I':
                opts.I_flag = true;
                break;
            case 'S':
                opts.S_flag = true;
                break;
            case 'Q':
                opts.Q_flag = true;
                break;
#ifdef HAVE_PCRE
            case 'P':
                opts.P_flag = true;
                break;
#endif
            /*case 'e':
                opts.e_flag = true;
                break;*/
            case 'V':
                puts(PACKAGE_VERSION);
                exit(1);
                break;
            case 'X':
                opts.X_flag = true;
                break;
            case 'H':
                opts.H_flag = true;
                break;
            case 'l':
                opts.l_flag = true;
                break;
            case 'L':
                opts.L_flag = true;
                break;
            case 'C':
                opts.C_flag = true;
                break;
            case 'r':
                opts.r_flag = true;
                break;
            case 'v':
                opts.v_flag = true;
                break;
            case '1':
                opts.one_flag = true;
                break;
            case '2':
                opts.two_flag = true;
                break;
            case 'z':
                break;
            case 'j':
                fprintf(stderr, "\n\nERROR: The -j option has been removed, unfortunately parsing bzip2 files is no longer supported\n\n");
            case 'h':
            default:
                usage(usage_msg);
                break;
        }
    }
    if((opts.P_flag | opts.G_flag | opts.E_flag) & opts.r_flag) {
        fprintf(stderr,
            "\n\nERROR: Searching for the reverse complement of a regular\n"
            "expression does not work. If your pattern is a litteral string just\n"
            "drop"
#ifdef HAVE_PCRE
            "the P, "
#endif
            "G or E flags. If your pattern is a regular expression\n"
            "you will have to manually input it as either a separate command or\n"
            "using alternation in the regular expression\n\n");
            usage(usage_msg);
    }
    return optind;
}

void split( std::vector<std::string> & theStringVector,  /* Altered/returned value */
       std::string theString,
       const  std::string theDelimiter)
{
    assert(theDelimiter.size() > 0);

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

void tokenizePatternFile(std::ifstream& in) {
    // tokenize a line from the pattern file.  The first part will be the pattern and the second
    // part is the file to write to.

    std::string lineptr;

    while(in.good()) {
        std::getline(in, lineptr);
        if(lineptr.empty()) {
            continue;
        }
        std::vector<std::string> fields;
        split(fields, lineptr, "\t");
        switch(fields.size()) {
            case 0:
                break;
            case 1:
                manager.add(fields[0]);
                if(opts.r_flag) {
                    std::string rcpattern = fields[0];
                    reverseComplement(rcpattern);
                    manager.add(rcpattern);
                }
                break;
            default:
                manager.add(fields[0], fields[1]);
                if(opts.r_flag) {
                    std::string rcpattern = fields[0];
                    reverseComplement(rcpattern);
                    manager.add(rcpattern, fields[1]);
                }
                break;
        }
    }
}

static int on_match(FILE * out, ReadPair& pair) {
    if(opts.l_flag)
    {
        return 1;
    }

    if(opts.c_flag) {
        ++matched_records;
        return 0;
    }

    pair.print(out);
    return  0;
}

static int on_match2(int strnum, const char *textp, void const * context) {
    if(opts.v_flag) {
        return 1;
    }
    if(opts.l_flag)
    {
        return 1;
    }
    if(opts.c_flag) {
        ++matched_records;
        return 1;
    }
    ReadPair * pair = (ReadPair *) context;
    FILE * out = manager.find(pattv[strnum].ptr);
    pair->print(out);
    return  1;
}

static std::string * prepare_data(Options & opts, kseq& read) {
  if(opts.H_flag) {
    return &read.name;

  } else if(opts.Q_flag){
    if(!read.isFasta()) {
      return &read.qual;
    }
  } else if (opts.C_flag) {
    return &read.comment;
  }

  return &read.seq;
}

int hash_search(Fxstream& stream) {


    ReadPair pair;
    std::map<std::string, int>::iterator iter;
    while(stream.read(pair) == 0) {

        std::string * data = prepare_data(opts, pair.first);

        FILE * out = manager.find(*data);
        if(out != NULL) {
            if(! opts.v_flag) {
                if(on_match(out,  pair) == 1)
                {
                    // the l flag has been set, print the file name
                    printf("%s\n", stream.getFile1().c_str());
                    return 0;
                }
            }

        } else {
            // read one did not have a match check read 2 if it exists
            if(!pair.second.empty()) {
                data = prepare_data(opts, pair.second);
                FILE * out = manager.find(*data);
                if(out != NULL) {
                    if(on_match(out,  pair) == 1)
                    {
                        // the l flag has been set, print the file name
                        printf("%s\n", stream.getFile2().c_str());
                        return 0;
                    }
                } else if (opts.v_flag) {
                    on_match(stdout, pair);
                }
            } else if (opts.v_flag) {
                on_match(stdout, pair);
            }
        }
        pair.clear();
    }

    return 0;
}

int multipattern_search(Fxstream& stream) {


    ReadPair pair;

    std::string conc;
    std::map<std::string, int>::iterator iter;
    for (iter = manager.patternMapping.begin(); iter != manager.patternMapping.end(); ++iter ) {
        conc += iter->first + "\n";
    }
    char * concstr = new char[conc.size() + 1];
    std::copy(conc.begin(), conc.end(), concstr);
    concstr[conc.size()] = '\0';

    // this is a hack as refsplit creates an extra blank record, which stuffs
    // up the search if the string ends with a new line character. Basically
    // here I'm replacing the last newline with null to prevent this
    concstr[conc.size()-1] = '\0';
    int npatts;

    pattv = refsplit(concstr, '\n', &npatts);

    SSEARCH *ssp = ssearch_create(pattv, npatts);

    if(ssp == NULL) {
        fprintf(stderr, "problem initalizing multi-search\n");
        /*for(int i = 0;i < npatts; ++i) {
            fprintf(stderr, "%s %d\n", pattv[i].ptr, pattv[i].len);
        }
        fprintf(stderr, "came from std::string:\n%s\n%d\n", conc.c_str(),conc.size());
        fprintf(stderr, "generated from manager strings:\n");
        for (iter = manager.patternMapping.begin(); iter != manager.patternMapping.end() ; ++iter ) {
            fprintf(stderr, "%s\n", iter->first.c_str());
        }*/
        return 1;
    }

    while(stream.read(pair) == 0) {
        MEMREF data;
        if(opts.H_flag) {
            data.ptr = pair.first.name.c_str();
            data.len = pair.first.name.size();

        } else if(opts.Q_flag){
            if(!pair.first.isFasta()) {
                data.ptr = pair.first.qual.c_str();
                data.len = (size_t)pair.first.length();
            }
        } else if (opts.C_flag){
            data.ptr = pair.first.comment.c_str();
            data.len = pair.first.comment.size();
        } else {
            data.ptr = pair.first.seq.c_str();
            data.len = (size_t)pair.first.length();
        }
        int ret = ssearch_scan(ssp, data, (SSEARCH_CB)on_match2, &pair);
        if(! ret) {
            // read one did not have a match check read 2 if it exists
            if(!pair.second.empty()) {
                if(opts.H_flag) {
                    data.ptr = pair.second.name.c_str();
                    data.len = pair.second.name.size();

                } else if(opts.Q_flag){
                    if(!pair.second.isFasta()) {
                        data.ptr = pair.second.seq.c_str();
                        data.len = (size_t)pair.second.length();
                    }
                } else if (opts.C_flag){
                  data.ptr = pair.second.comment.c_str();
                  data.len = pair.second.comment.size();
                } else {
                    data.ptr = pair.second.seq.c_str();
                    data.len = (size_t)pair.second.length();
                }
                ret = ssearch_scan(ssp, data, (SSEARCH_CB)on_match2, &pair);
                if(!ret && opts.v_flag) {
                    on_match(stdout, pair);
                } else if (opts.l_flag)
                {
                    // read 2 matched and the l flag is in place, print file name
                    printf("%s\n", stream.getFile2().c_str());
                    return 0;

                }
            } else if (opts.v_flag) {
                on_match(stdout, pair);
            }
        } else if (opts.l_flag)
        {
            // read 1 matched and l option in place, print file name
            printf("%s\n", stream.getFile1().c_str());
            return 0;
        }
        pair.clear();
    }

    delete [] concstr;
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

int posix_regex_search(Fxstream& stream, regex_t * pxr) {


    ReadPair pair;

    while(stream.read(pair) == 0) {
        std::string * data = prepare_data(opts, pair.first);

        int ret = regexec(pxr, data->c_str(), 0, NULL, 0);
        if(ret == REG_NOMATCH){
            // read one did not have a match check read 2 if it exists
            if(!pair.second.empty()) {
                data = prepare_data(opts, pair.second);
                ret = regexec(pxr, data->c_str(), 0, NULL, 0);
                if(ret == REG_NOMATCH && opts.v_flag) {
                    on_match(stdout, pair);
                }
                if(ret != (REG_NOMATCH | 0)) {
                    char * errorbuf = get_regerror(ret, pxr);
                    fprintf(stderr, "%s\n", errorbuf);
                    free(errorbuf);
                    return 1;
                } else if(ret == 0) {
                    if(on_match(stdout, pair) == 1)
                    {
                        // the l flag has been set, print the file name
                        printf("%s\n", stream.getFile2().c_str());
                        return 0;
                    }

                }
            } else if (opts.v_flag) {
                on_match(stdout, pair);
            }
        } else if(ret != 0) {
            char * errorbuf = get_regerror(ret, pxr);
            fprintf(stderr, "%s\n", errorbuf);
            free(errorbuf);
            return 1;
        } else if(! opts.v_flag ){
            if(on_match(stdout, pair) == 1)
            {
                // the l flag has been set, print the file name
                printf("%s\n", stream.getFile1().c_str());
                return 0;
            }

        }
        pair.clear();
    }

    return 0;
}
#ifdef HAVE_PCRE
int pcre_search(Fxstream& stream, pcre * pxr) {
    ReadPair pair;
    int ovector[30];

    while(stream.read(pair) == 0) {
        std::string * data = prepare_data(opts, pair.first);
        int ret = pcre_exec(pxr, NULL, data->c_str(), data->size(), 0, 0, ovector, 30);
        if(ret != 1){
            // read one did not have a match check read 2 if it exists
            if(!pair.second.empty()) {
                data = prepare_data(opts, pair.second);
                ret = pcre_exec(pxr, NULL, data->c_str(), data->size(), 0, 0, ovector, 30);
                if(ret == 1) {
                    // second read had a match
                    if(on_match(stdout, pair) == 1)
                    {
                        // the l flag has been set, print the file name
                        printf("%s\n", stream.getFile2().c_str());
                        return 0;
                    }
                } else if(opts.v_flag) {
                    on_match(stdout, pair);
                }
            } else if(opts.v_flag) {
                on_match(stdout, pair);
            }
        } else if (!opts.v_flag){
            // read one matched
            if(on_match(stdout, pair) == 1)
            {
                // the l flag has been set, print the file name
                printf("%s\n", stream.getFile1().c_str());
                return 0;
            }

        }
        pair.clear();
    }
    return 0;
}
#endif
std::string prepare_revcomp(const char * pattern) {
    std::string rcpattern;
    if(opts.r_flag ^ (opts.H_flag | opts.Q_flag | opts.C_flag))
    {
        rcpattern = pattern;
        reverseComplement(rcpattern);
    }
    return rcpattern;
}

// returns true if the pattern matches the data
bool simple_string_search_core(const char * pattern, const char * rcpattern, const char * haystack)

{
    const char * ret = strstr(haystack, pattern);
    if(ret == NULL)
    {
        if(rcpattern != NULL)
        {
            ret = strstr(haystack, rcpattern);
            if(ret != NULL)
            {
                return true;
            }
        }
    }
    else
    {
        return true;
    }
    return false;
}

bool after_match()
{
    return true;

}

int simple_string_search(Fxstream& stream, const char * pattern) {
    ReadPair pair;
    std::string rcpattern = prepare_revcomp(pattern);
    while(stream.read(pair) == 0) {
        bool read_matched = false;
        std::string * data = prepare_data(opts, pair.first);
        if(rcpattern.empty()) {
            read_matched = simple_string_search_core(pattern, NULL, data->c_str());
        } else {
            read_matched = simple_string_search_core(pattern, rcpattern.c_str(), data->c_str());
        }

        if(read_matched) {
            if(! opts.v_flag) {
                // the first read has a match
                if(on_match(stdout, pair) == 1) {
                    // the l flag has been set, print the file name
                    printf("%s\n", stream.getFile1().c_str());
                    return 0;
                }
            }
        } else {
            if(opts.v_flag) {
                on_match(stdout, pair);
            } else {
                // read one did not have a match check read 2 if it exists
                if(!pair.second.empty()) {
                    data = prepare_data(opts, pair.second);
                    if(rcpattern.empty()) {
                        read_matched = simple_string_search_core(pattern, NULL, data->c_str());

                    } else {
                        read_matched = simple_string_search_core(pattern, rcpattern.c_str(), data->c_str());
                    }

                    if(read_matched) {
                        if(! opts.v_flag) {
                            // the first read has a match
                            if(on_match(stdout, pair) == 1) {
                                // the l flag has been set, print the file name
                                printf("%s\n", stream.getFile1().c_str());
                                return 0;
                            }
                        }
                    } else {
                        if(opts.v_flag) {
                            on_match(stdout, pair);
                        }
                    }
                }
            }
        }
        pair.clear();
    }
    return 0;

}

int main(int argc, char * argv[])
{
    int opt_idx = parseOptions(argc, argv);
    std::string pattern;
    if(opts.f_flag == NULL) {

        if( opt_idx >= argc) {
            puts("Please provide a pattern (or pattern file) and at least one input file");
            usage(usage_msg);
        } else if (opt_idx >= argc - 1) {
            puts("Please provide an input file");
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
            puts("Please provide an input file");
            usage(usage_msg);
        }
        std::ifstream in (opts.f_flag);
        if(!in.good()) {
            fprintf(stderr, "problem opening pattern file\n");
            exit(1);
        } else {
            tokenizePatternFile(in);
        }
    }

    while(opt_idx < argc) {
        Fxstream stream;
        int stream_state = 1;
        if(! opts.S_flag && ! opts.I_flag) {
            // two read files
            stream_state = stream.open(argv[opt_idx], argv[opt_idx+1], opts.I_flag);

        } else {
            // one read file
            stream_state = stream.open(argv[opt_idx], NULL, opts.I_flag);
        }
        if(stream_state != 0) {
            fprintf(stderr, "Failed to open stream\n");
            return 1;
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
                return 1;
            }
            ret = posix_regex_search(stream, &px_regex);
            regfree(&px_regex);
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

            pcre_search(stream, re);
            pcre_free(re);
        }
#endif
        else if(opts.X_flag) {
            hash_search(stream);
        }
        else if(opts.f_flag) {
            multipattern_search(stream);

        } else {
            simple_string_search(stream, pattern.c_str());
        }

        if(opts.c_flag) {
            printf("%d\n", matched_records);
        }

        if(! opts.S_flag && ! opts.I_flag) {
            opt_idx+=2;
        } else {
            opt_idx++;
        }
    }
    return 0;
}
