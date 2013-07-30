//
//  main.cpp
//  fxtract
//
//  Created by Connor Skennerton on 30/07/13.
//  Copyright (c) 2013 Connor Skennerton. All rights reserved.
//

#include <fstream>
#include <iostream>
#include <unistd.h>

#include <seqan/seq_io.h>
#include <seqan/sequence.h>
#include <seqan/find.h>
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
    
    Options() :
    gzip(false), bzip2(false), fasta(false), fastq(false)
    {}
};

typedef seqan::Pattern<seqan::String<seqan::CharString>, seqan::WuManber> WuMa;


void printSingle(Fx& mate1 ) {
    if (seqan::empty(mate1.qual)) {
        std::cout<<">"<<mate1.id<<std::endl;
        std::cout<<mate1.seq<<std::endl;
        //seqan::writeRecord(std::cout, mate1.id, mate1.seq, seqan::Fasta());
        
    } else {
        std::cout<<"@"<<mate1.id<<'\n'<<mate1.seq<<"\n+\n"<<mate1.qual<<std::endl;
        //seqan::writeRecord(std::cout, mate1.id, mate1.seq, mate1.qual, seqan::Fastq());
    }
}

void printPair(Fx& mate1, Fx& mate2) {
    // match in the first read print out pair
    printSingle(mate1);
    printSingle(mate2);
}

void usage() {
    std::cout<< "fxtract [-zjq] pattern read1.fx [read2.fx]\n";
    std::cout<< "\t-z\t\tForce gzip formatting\n";
    std::cout<<"\t-j\t\tForce bzip2 formatting\n";
    std::cout<<"\t-q\t\tForce fastq formatting\n";
    std::cout<<"\ndefault is auto detection of the above parameters"<<std::endl;
    exit(1);
}

int parseOptions(int argc,  char * argv[], Options& opts) {
    int c;
    while ((c = getopt(argc, argv, "hf:zjq")) != -1 ) {
        switch (c) {
            case 'f':
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
            case 'h':
            default:
                usage();
                break;
        }
    }
    return optind;
}


int main(int argc, char * argv[])
{
    Options opts;
    int opt_idx = parseOptions(argc, argv, opts);
    if (opt_idx >= argc - 1) {
        std::cout<< "Please provide a pattern and at least one input file"<<std::endl;
        usage();
    }
    if (opt_idx >= argc) {
        std::cout << "Please provide at least one input file"<<std::endl;
        usage();
    }
    seqan::CharString pattern = argv[opt_idx++];
    seqan::CharString rcpattern = pattern;
    seqan::reverseComplement(rcpattern);
    
    seqan::String<seqan::String<char> > pattern_list;
    seqan::appendValue(pattern_list, pattern);
    seqan::appendValue(pattern_list, rcpattern);
    
    
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
            
            seqan::Finder<seqan::CharString> finder(mate1.seq);
            if (seqan::find(finder, needle)) {
                printPair(mate1, mate2);

            } else {
                seqan::Finder<seqan::CharString> finder(mate2.seq);
                //seqan::setHaystack(finder, mate2.seq);
                if (seqan::find(finder, needle)) {
                    printPair(mate1, mate2);
                }
                
            }
        }
    } else {
        while (!atEnd(read1))
        {
            if (readRecord(mate1.id, mate1.seq, mate1.qual, read1) != 0) {
                std::cerr<<"Malformed record"<<std::endl;
            }
            seqan::Finder<seqan::CharString> finder(mate1.seq);
            if (seqan::find(finder, needle)) {
                printSingle(mate1);
            }
        }
    }
    
    return 0;
    
    
}

