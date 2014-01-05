/*
 * fileManager.cpp
 * Copyright (C) 2013 uqcskenn <uqcskenn@hawke>
 *
 * Distributed under terms of the MIT license.
 */

#include <seqan/sequence.h>
#include <seqan/modifier.h>
#include <sys/syslimits.h>
#include "fileManager.h"

FileManager::FileManager(std::map<seqan::CharString, seqan::CharString>& mapping){
    std::map<seqan::CharString, seqan::CharString>::iterator it;
    for (it = mapping.begin(); it != mapping.end(); it++) {
        if(seqan::empty(it->second)) {
            add(it->first);
            //throw FileManagerException("You must have a file associated with every pattern");
        } else {
            add(it->first, it->second);
        }
    }
}

FileManager::FileManager(std::vector<seqan::CharString>& mapping) {
    std::vector<seqan::CharString>::iterator it;
    for (it = mapping.begin(); it != mapping.end(); ++it) {
        add(*it);
    }
}

FileManager::~FileManager() {
    fpointer_t::iterator it;
    for(it = mOutfiles.begin(); it != mOutfiles.end(); it++) {
        it->close();
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

        FileWrapper fw = FileWrapper();
        fw.filename = seqan::toCString(filename);
        fw.deleteAtEnd = true;
        
        mOutfiles.push_back(fw);

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
    
    FileWrapper fw = FileWrapper();
    fw.file = stdout;
    fw.deleteAtEnd = false;
    fw.fileOpened = true;
    mOutfiles.push_back(fw);

    // get the sequence and its reverse complement
    // pointing to the output file
    seqan::CharString rc = pattern;
    seqan::reverseComplement(rc);
    mMapping[rc] = mMapping[pattern];
}


FILE * FileManager::operator[] (seqan::CharString key) {
    if (openCount >= OPEN_MAX) {
        mOutfiles.at(mOpened.front()).close();
        mOpened.pop();
        openCount--;
    }
    mOutfiles.at(mMapping[key]).open();
    mOpened.push(mMapping[key]);
    return mOutfiles.at(mMapping[key]).file;
}
