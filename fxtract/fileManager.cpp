/*
 * fileManager.cpp
 * Copyright (C) 2013 uqcskenn <uqcskenn@hawke>
 *
 * Distributed under terms of the MIT license.
 */

#include <seqan/sequence.h>
#include <seqan/modifier.h>

#include "fileManager.h"

FileManager::FileManager(std::map<seqan::CharString, seqan::CharString>& mapping){
    std::map<seqan::CharString, seqan::CharString>::iterator it;
    for (it = mapping.begin(); it != mapping.end(); it++) {
        if(seqan::empty(it->second)) {
            throw FileManagerException("You must have a file associated with every pattern");
        }
        add(it->first, it->second);
    }
    noDelete = false;
}

FileManager::FileManager(std::vector<seqan::CharString>& mapping) {
    std::vector<seqan::CharString>::iterator it;
    for (it = mapping.begin(); it != mapping.end(); ++it) {
        add(*it);
    }
    noDelete = true;
}

FileManager::~FileManager() {
    if(! noDelete) {
        fpointer_t::iterator it;
        for(it = mOutfiles.begin(); it != mOutfiles.end(); it++) {
            delete *it;
        }
    }
}

fmapping_t::iterator FileManager::begin() {
    return mMapping.begin();
}

fmapping_t::iterator FileManager::end() {
    return mMapping.end();
}

fmapping_t::iterator FileManager::find(seqan::CharString key) {
    return mMapping.find(key);
}

void FileManager::add(seqan::CharString pattern, seqan::CharString filename) {
    fmapping_t::iterator it = mFilenameMapping.find(filename);
    if (it == mFilenameMapping.end()) {
    
        mMapping[pattern] = mOutfiles.size();
        mFilenameMapping[filename] = mMapping[pattern];
        std::ofstream * out = new std::ofstream(seqan::toCString(filename));
        mOutfiles.push_back(out);

        if(! out->good()) {
            seqan::CharString msg = "cannot open file ";
            msg += filename;
            throw FileManagerException(seqan::toCString(msg));
        }
        // get the sequence and its reverse complement
        // pointing to the output file
        seqan::CharString rc = pattern;
        seqan::reverseComplement(rc);
        mMapping[rc] = mMapping[pattern];
    } else {
        mMapping[pattern] = it->second;
        seqan::CharString rc = pattern;
        seqan::reverseComplement(rc);
        mMapping[rc] = mMapping[pattern];
    }
}

void FileManager::add(seqan::CharString pattern) {
        mMapping[pattern] = mOutfiles.size();
        std::ostream * out = new std::ofstream();
        out->rdbuf(std::cout.rdbuf());
        mOutfiles.push_back(out);

        if(! out->good()) {
            seqan::CharString msg = "cannot bind cout";
            throw FileManagerException(seqan::toCString(msg));
        }
        // get the sequence and its reverse complement
        // pointing to the output file
        seqan::CharString rc = pattern;
        seqan::reverseComplement(rc);
        mMapping[rc] = mMapping[pattern];
    noDelete = true;
}


std::ostream& FileManager::operator[] (seqan::CharString key) {
    return *(mOutfiles.at(mMapping[key]));
}
