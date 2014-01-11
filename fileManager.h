/*
 * fileManager.h
 *
 * Store a mapping of strings to FILE for printing to muliple files
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
#include <queue>
#include <cstdio>
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

struct FileWrapper {
    FILE * file;
    char * filename;
    bool fileOpened;
    bool deleteAtEnd;
    
    FileWrapper(const char * _name) : file(NULL), fileOpened(false), deleteAtEnd(false)
    {
        if(_name != NULL)
            filename = strdup(_name);
        else
            filename = NULL;
    }
    ~FileWrapper() 
    {
        close();
        free(filename);
    }

    void open();
    void close();
};

typedef std::vector<FileWrapper *>fpointer_t;
typedef std::map<seqan::CharString, int > fmapping_t;

class FileManager
{
public:
    FileManager() : openCount(0){}
    FileManager(std::map<seqan::CharString, seqan::CharString>& mapping);
    FileManager(std::vector<seqan::CharString>& mapping);
    ~FileManager();
    fmapping_t::iterator begin();
    fmapping_t::iterator end();
    fmapping_t::iterator find(seqan::CharString key);
    void add(seqan::CharString key, seqan::CharString file_name);
    void add(seqan::CharString key);
    FILE * operator[]( seqan::CharString key);

    fpointer_t mOutfiles;
private:
    bool mkdir_p(const char * filename);

    int openCount;
    fmapping_t mMapping;
    fmapping_t mFilenameMapping;
    std::queue<int> mOpened;
};


#endif /* !__FILEMANAGER_H__ */

