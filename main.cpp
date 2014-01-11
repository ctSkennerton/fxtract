//
//  main.cpp
//  fxtract
//
//  Created by Connor Skennerton on 30/07/13.
//  Copyright (c) 2013 Connor Skennerton. All rights reserved.
//

#include <fstream>
#include <iostream>
#include <set>
#include <vector>
#include <string>
#include <unistd.h>

#include <seqan/seq_io.h>
#include <seqan/sequence.h>
#include <seqan/find.h>

#include "util.h"
#include "fileManager.h"


#define VERSION "0.4"

struct Fx
{
    seqan::CharString id;
    seqan::CharString seq;
    seqan::CharString qual;
    
    Fx()
    {};
    
    Fx(seqan::CharString _id, seqan::CharString _seq, seqan::CharString _qual = "") :
    id(_id), seq(_seq), qual(_qual){}
};


struct Options
{
    bool gzip;
    bool bzip2;
    bool fasta;
    bool fastq;
    bool header;
    char * pattern_file;
    Options() :
    gzip(false), bzip2(false), fasta(false), fastq(false), header(false), pattern_file(NULL)
    {}
};

typedef seqan::Pattern<seqan::String<seqan::CharString>, seqan::WuManber> WuMa;


void printSingle(Fx& mate1, FILE * out ) {
    if (seqan::empty(mate1.qual)) {
        //out<<">"<<mate1.id<<std::endl;
        //out<<mate1.seq<<std::endl;
        seqan::writeRecord(out, mate1.id, mate1.seq, seqan::Fasta());
        
    } else {
        //std::cout<<"@"<<mate1.id<<'\n'<<mate1.seq<<"\n+\n"<<mate1.qual<<std::endl;
        seqan::writeRecord(out, mate1.id, mate1.seq, mate1.qual, seqan::Fastq());
    }
}

void printPair(Fx& mate1, Fx& mate2, FILE * out) {
    // match in the first read print out pair
    printSingle(mate1, out);
    printSingle(mate2, out);
}

void usage() {
    std::cout<< "fxtract [-hHv] -f pattern_file | pattern <read1.fx> [<read2.fx>]\n";
    std::cout<<"\t-H           Evaluate patterns in the context of headers (default: sequences)\n";
    //std::cout<<"\t-j           Force bzip2 formatting\n";
    //std::cout<<"\t-q           Force fastq formatting\n";
    std::cout<<"\t-f <file>    File containing patterns, one per line" <<std::endl;
    std::cout<<"\t-h           Print this help"<<std::endl;
    std::cout<<"\t-V           Print version"<<std::endl;
    exit(1);
}

int parseOptions(int argc,  char * argv[], Options& opts) {
    int c;
    while ((c = getopt(argc, argv, "Hhf:zjqV")) != -1 ) {
        switch (c) {
            case 'f':
                opts.pattern_file = optarg;
                break;
            case 'z':
                opts.gzip = true;
                break;
            case 'j':
                opts.bzip2 = true;
                break;
            case 'q':
                opts.fastq = true;
                break;
            case 'V':
                std::cout <<VERSION<<std::endl;
                exit(1);
                break;
            case 'H':
                opts.header = true;
                break;
            case 'h':
            default:
                usage();
                break;
        }
    }
    return optind;
}

void tokenizePatternFile(std::istream& in, FileManager& fmanager) {
    // tokenize a line from the pattern file.  The first part will be the pattern and the second
    // part is the file to write to.
    std::map<seqan::CharString, seqan::CharString> results;
    std::vector<std::string> fields;
    std::string line;

    while(std::getline(in, line)) {
        tokenize(line, fields);
        switch(fields.size()) {
            case 0:
                break;
            case 1:
                results[fields[0]] = "";
                break;
            default:
                
                if(results.find(fields[0]) != results.end()) {
                    // patterns are the same
                    if(results[fields[0]] != fields[1]) {
                        // warn user if it was supposed to go to a different file
                        std::cerr << "pattern "<< fields[0] << " not unique but different output file requested: " << results[fields[0]] <<" and "<< fields[1] <<std::endl;
                        std::cerr << "output will only go into " <<results[fields[0]]<<std::endl;
                    }
                } else {
                    results[fields[0]] = fields[1];
                }
                break;
        }
        fields.clear();
    }
    fields.clear();
    
    std::map<seqan::CharString, seqan::CharString>::iterator it;
    for(it = results.begin(); it != results.end(); ++it) {
        if(it->second == "") {
            fmanager.add(it->first);
        } else {
            fmanager.add(it->first, it->second);
        }
    }
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
    
    try { 
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
    } catch(FileManagerException& e) {
        std::cerr << e.what()<<std::endl;
        return 1;
    }
    return 0;
    
    
}

