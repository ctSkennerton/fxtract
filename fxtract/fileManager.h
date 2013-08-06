/*
 * fileManager.h
 *
 * Store a mapping of strings to std::ofstream for printing to muliple files
 * all at once without needing to open and close every file for each write
 *
 * Copyright (C) 2013 uqcskenn <uqcskenn@hawke>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef __FILEMANAGER_H__
#define __FILEMANAGER_H__

#include <map>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <exception>

#include <seqan/sequence.h>

class FileManagerException : std::exception {
public:
    FileManagerException(const char * _msg) : msg(_msg) {}
    const char* what() {
        return msg;
    }
private:
    const char * msg;
};

typedef std::vector<std::ostream* >fpointer_t;
typedef std::map<seqan::CharString, int > fmapping_t;

class FileManager
{
public:
    FileManager() : noDelete(false){}
    FileManager(std::map<seqan::CharString, seqan::CharString>& mapping);
    FileManager(std::vector<seqan::CharString>& mapping);
    ~FileManager();
    fmapping_t::iterator begin();
    fmapping_t::iterator end();
    fmapping_t::iterator find(seqan::CharString key);
    void add(seqan::CharString key, seqan::CharString file_name);
    void add(seqan::CharString key);
    std::ostream& operator[]( seqan::CharString key);

     fpointer_t mOutfiles;
private:
     fmapping_t mMapping;
     bool noDelete;
};


#endif /* !__FILEMANAGER_H__ */

