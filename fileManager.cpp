/*
 * fileManager.cpp
 * Copyright (C) 2013 uqcskenn <uqcskenn@hawke>
 *
 * Distributed under terms of the MIT license.
 */

#include <seqan/sequence.h>
#include <seqan/seq_io.h>
#include <seqan/modifier.h>
#include "fileManager.h"
#define OPEN_MAX 256

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
        delete (*it);
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

        FileWrapper * fw = new FileWrapper(seqan::toCString(filename));
        fw->deleteAtEnd = true;
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
    
    FileWrapper * fw = new FileWrapper(NULL);
    fw->file = stdout;
    fw->deleteAtEnd = false;
    fw->fileOpened = true;
    mOutfiles.push_back(fw);

    // get the sequence and its reverse complement
    // pointing to the output file
    seqan::CharString rc = pattern;
    seqan::reverseComplement(rc);
    mMapping[rc] = mMapping[pattern];
}


FILE * FileManager::operator[] (seqan::CharString key) {
    //std::cerr<< "key: "<< key<<std::endl;
    //std::cerr << "Mapping key: "<<mMapping[key]<<std::endl;
    FileWrapper * fw = mOutfiles.at(mMapping[key]);
    //std::cerr << "attempting to write to file: "<<fw->filename<<std::endl;
    if(!fw->fileOpened) {
        if (openCount >= OPEN_MAX) {
            FileWrapper * last_opened = mOutfiles.at(mOpened.front());
            //std::cerr<<"max opened file limit reached: "<<OPEN_MAX<<std::endl;
            //std::cerr<<"closing "<<last_opened->filename<<"temporarily to make room for more"<<std::endl;
            last_opened->close();
            mOpened.pop();
            openCount--;
        }
        fw->open();
        mOpened.push(mMapping[key]);
    }
    return fw->file;
    //return stdout;
}
