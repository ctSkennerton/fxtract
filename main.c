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

#define VERSION "1.0-alpha"

typedef struct _Options
{
    bool   gzip;
    bool   bzip2;
    bool   fasta;
    bool   fastq;
    bool   header;
    char * pattern_file;
} Options;

KHASH_INIT(s2s, sds, sds, 1, kh_str_hash_func, kh_str_hash_equal)

void options_init(Options * opts) {
    opts->gzip = false; opts->bzip2 = false; opts->fasta = false; opts->fastq = false; opts->header = false; opts->pattern_file = NULL;
}


void printSingle(Fx * mate1, FILE * out ) {
    if (mate1->qual_start) {
        fputc('@', out);
        fwrite(mate1->data + mate1->header_start, 1, mate1->header_len, out);
        fputc('\n', out);
        fwrite(mate1->data + mate1->seq_start, 1, mate1->seq_len, out);
        fputs("\n+\n", out);
        fwrite(mate1->data + mate1->qual_start, 1, mate1->qual_len, out);
        fputc('\n', out);
    } else {
        fputc('>', out);
        fwrite(mate1->data + mate1->header_start, 1, mate1->header_len, out);
        fputc('\n', out);
        fwrite(mate1->data + mate1->seq_start, 1, mate1->seq_len, out);
        fputc('\n', out);
    }
}

void printPair(Fx* mate1, Fx* mate2, FILE * out) {
    // match in the first read print out pair
    printSingle(mate1, out);
    printSingle(mate2, out);
}

void usage() {
    puts("fxtract [-hHv] -f pattern_file | pattern <read1.fx> [<read2.fx>]");
    puts("\t-H           Evaluate patterns in the context of headers (default: sequences)");
    //puts("\t-j           Force bzip2 formatting");
    //puts("\t-q           Force fastq formatting");
    puts("\t-f <file>    File containing patterns, one per line");
    puts("\t-h           Print this help");
    puts("\t-V           Print version");
    exit(1);
}

int parseOptions(int argc,  char * argv[], Options* opts) {
    int c;
    while ((c = getopt(argc, argv, "Hhf:zjqV")) != -1 ) {
        switch (c) {
            case 'f':
                opts->pattern_file = optarg;
                break;
            case 'z':
                opts->gzip = true;
                break;
            case 'j':
                opts->bzip2 = true;
                break;
            case 'q':
                opts->fastq = true;
                break;
            case 'V':
                puts(VERSION);
                exit(1);
                break;
            case 'H':
                opts->header = true;
                break;
            case 'h':
            default:
                usage();
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
                /*k = kh_get(s2s, results, split_line[0]);
                if(k == kh_end(results)) {
                    k = kh_put(s2s, results, split_line[0], &ret);
                    kh_value(results, k) = sdsdup(split_line[1]);
                } else if (strcmp(split_line[1], kh_value(results, k))) {
                    fprintf(stderr, "pattern \"%s\" is not unique but different output file requested: \"%s\" vs \"%s\"\n the output will go to %s\n", split_line[0], kh_value(results, k), split_line[1], kh_value(results, k));
                }*/
 
                break;
        }
        sdsfreesplitres(split_line, fields);
    }
    free(lineptr);
}

int main(int argc, char * argv[])
{
    
    seqan::String<seqan::String<char> > pattern_list;
    FileManager manager;
    Options opts;

    int opt_idx = parseOptions(argc, argv, opts);

    if(opts.pattern_file == NULL) {

        if( opt_idx >= argc) {
            std::cout<< "Please provide a pattern (or pattern file) and at least one input file"<<std::endl;
            usage();
        } else if (opt_idx >= argc - 1) {
            std::cout << "Please provide an input file (or two)" <<std::endl;
            usage();
        }
        seqan::CharString pattern = argv[opt_idx++];
        seqan::CharString rcpattern = pattern;
        seqan::reverseComplement(rcpattern);

        manager.add(pattern);
        
        seqan::appendValue(pattern_list, pattern);
        seqan::appendValue(pattern_list, rcpattern);

    } else {
        if (opt_idx > argc - 1) {
            std::cout << "Please provide an input file (or two)"<<std::endl;
            usage();
        }
        std::ifstream in(opts.pattern_file);
        try{
            tokenizePatternFile(in, manager);
        } catch(FileManagerException& e) {
            std::cerr << e.what() <<std::endl;
            return 1;
        }
        fmapping_t::iterator it;
        for(it = manager.begin(); it != manager.end(); ++it) {
            //std::cout << "pattern: "<<it->first<<" bound to index: "<<it->second<<std::endl;
            seqan::appendValue(pattern_list, it->first);
        }
    }
    typedef std::set<seqan::CharString> LookupTable;
    LookupTable lookup;
    if (opts.header) {

        typedef seqan::Iterator<seqan::String<seqan::CharString> >::Type TStringSetIterator;
        for (TStringSetIterator it = begin(pattern_list); it != end(pattern_list); ++it) {
              lookup.insert( value(it) );
        }
    }

    WuMa needle(pattern_list);
    
    
    seqan::SequenceStream read1(argv[opt_idx++]);
    if (!isGood(read1))
        std::cerr << "Could not open read1 file\n";
    // Read one record.
    Fx mate1 = Fx();
    if (opt_idx < argc) {
        // we have a mate file
        seqan::SequenceStream read2(argv[opt_idx]);
        if (!isGood(read2))
            std::cerr << "Could not open read2 file\n";
        Fx mate2 = Fx();

        while (!atEnd(read1))
        {
            if (atEnd(read2)) {
                std::cerr<< "files have different number of reads"<<std::endl;
                break;
            }

            
            if (readRecord(mate1.id, mate1.seq, mate1.qual, read1) != 0) {
                std::cerr<<"Malformed record"<<std::endl;
            }

            if (readRecord(mate2.id, mate2.seq, mate2.qual, read2) != 0) {
                std::cerr<<"Malformed record"<<std::endl;
            }
            if (opts.header) {
                seqan::StringSet<seqan::CharString> header_parts;
                seqan::strSplit(header_parts, mate1.id, ' ', false, 1);
                LookupTable::iterator pos = lookup.find(header_parts[0]);
                if(pos != lookup.end()) {
                    printPair(mate1, mate2, manager[header_parts[0]]);
                    lookup.erase(pos);
                    if(lookup.empty()) {
                        break;
                    }
                } else {
                    seqan::clear(header_parts);
                    seqan::strSplit(header_parts, mate2.id, ' ', false, 1);
                    if(lookup.find(header_parts[0]) != lookup.end()) {
                        printPair(mate1, mate2, manager[header_parts[0]]);
                        lookup.erase(pos);
                        if(lookup.empty()) {
                            break;
                        }
                    }
                }
            }
            else {
                seqan::Finder<seqan::CharString> finder(mate1.seq);
                if (seqan::find(finder, needle)) {

                    printPair(mate1, mate2, manager[pattern_list[seqan::position(needle)]]);

                } else {
                    seqan::Finder<seqan::CharString> finder(mate2.seq);
                    //seqan::setHaystack(finder, mate2.seq);
                    if (seqan::find(finder, needle)) {
                        printPair(mate1, mate2, manager[pattern_list[seqan::position(needle)]]);
                        //printPair(mate1, mate2, std::cout);
                    }
                    
                }
            }
        }
    } else {
        while (!atEnd(read1))
        {
            if (readRecord(mate1.id, mate1.seq, mate1.qual, read1) != 0) {
                std::cerr<<"Malformed record"<<std::endl;
            }
            if(opts.header) {
                seqan::StringSet<seqan::CharString> header_parts;

                seqan::strSplit(header_parts, mate1.id, ' ', false, 1);
                LookupTable::iterator pos = lookup.find(seqan::value(header_parts, 0));
                if(pos != lookup.end()) {
                    printSingle(mate1, manager[seqan::value(header_parts,0)]);
                    lookup.erase(pos);
                    if(lookup.empty()) {
                        break;
                    }
                }
            } else {
                seqan::Finder<seqan::CharString> finder(mate1.seq);
                if (seqan::find(finder, needle)) {
                    printSingle(mate1, manager[pattern_list[seqan::position(needle)]]);
                }
            }
        }
    }
    
    return 0;
    
    
}

