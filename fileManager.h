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

#include <cstdio>
#include <cstdlib>
#include <map>
#include <vector>
#include <queue>
#include <string>

#ifndef OPEN_MAX
#define OPEN_MAX 256
#endif

struct FileWrapper {
    FILE *                     file;           // our file
    std::string                filename;       // the filename that we are writting to
    bool                       fileOpened;     // is our file opened?
    bool                       closeAtEnd;     // should this FILE be closed when finished - not to be used for stdout, stderr
    size_t                     recordsWritten; // count of fasta/fastq records written to this file - used in priority queue comparison

    FileWrapper();
    FileWrapper(std::string filename);
    ~FileWrapper();
    int open();
    void close();

};

class Pqcomp {
    public:
    bool operator () (const FileWrapper * lhs, const FileWrapper * rhs) {
        return lhs->recordsWritten > rhs->recordsWritten;
    }
};

struct FileManager {
    int                        openCount;       // keep count of the number of files that we've opened
    std::map<std::string, int>         patternMapping;  // hash of patterns to indexes into the array of filewrappers
    std::map<std::string, int>         filenameMapping; // hash of filenames to indexes into the array of filewrappers
    std::vector<FileWrapper *> files;           // An array of all of the file wrappers that are being managed
    std::priority_queue<FileWrapper *, std::vector<FileWrapper *>, Pqcomp> closingQueue;          // priority queue used to determine which filewrapper should be closed first when there are too many

    FileManager();
    ~FileManager();
    void add(std::string key);
    void add(std::string key, std::string fileName);
    FILE * find(std::string key);
    int size(){return static_cast<int>(patternMapping.size());}
};


#endif /* !__FILEMANAGER_H__ */

