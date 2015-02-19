/*
 * fileManager.cpp
 * Copyright (C) 2013 uqcskenn <uqcskenn@hawke>
 *
 * Distributed under terms of the MIT license.
 */


#include "fileManager.h"
#include "util.h"
#include <cassert>
#include <iostream>


FileWrapper::FileWrapper() {
    file = NULL;
    filename = "";
    fileOpened = false;
    closeAtEnd = false;
    recordsWritten = 0;
}

FileWrapper::FileWrapper(std::string filename) {
    file = NULL;
    filename = filename;
    fileOpened = false;
    closeAtEnd = false;
    recordsWritten = 0;
}

int FileWrapper::open() {
    if(!fileOpened) {
        file = fopen(filename.c_str(), "a");
    }

    if (file == NULL) {
        perror("problem opening file");
        return 1;
    }
    fileOpened = true;
    return 0;
}


void FileWrapper::close() {
    if(fileOpened & closeAtEnd) {
        fclose(file);
        fileOpened = false;
    }
}

FileWrapper::~FileWrapper() {
    close();
}

FileManager::FileManager() {
    openCount = 0;
}

FileManager::~FileManager() {
    std::vector<FileWrapper *>::iterator iter2;
    for(iter2 = files.begin(); iter2 != files.end(); ++iter2) {
        if(*iter2 != NULL) {
            delete *iter2;
            *iter2 = NULL;
        }
    }
}



void FileManager::add(std::string pattern, std::string filename) {
    int ret;
    // check if the pattern has been seen before
    std::cerr << pattern << " : " << filename<<std::endl;
    std::map<std::string, int>::iterator pm_iter = patternMapping.find(pattern);
    std::map<std::string, int>::iterator fp_iter;
    if(pm_iter == patternMapping.end()) {

        std::string rc_pattern = pattern;
        reverseComplement(rc_pattern);
        // add in the pattern key to the hash
        patternMapping[pattern] = -1;
        patternMapping[rc_pattern] = -1;

        // now check to see if the filename has been seen before
        fp_iter = filenameMapping.find(filename);
        if (fp_iter == filenameMapping.end()) {
            // it hasn't so make a new filewrapper and associate the
            // pattern and the filename with it's index in the vector

            FileWrapper * fw = new FileWrapper(filename);
            // since the size value will always be one more than the index
            // after we push on the filewrapper below the index will match
            int n = files.size();
            patternMapping[pattern] = n;
            patternMapping[rc_pattern] = n;

            filenameMapping[filename] = n;

            fw->filename = filename;
            fw->closeAtEnd = true;
            files.push_back(fw);

        } else {
            // the file has been seen before but the pattern hasn't
            // so we need to associate the new pattern with the file
            patternMapping[pattern] = fp_iter->second;
            patternMapping[rc_pattern] = fp_iter->second;
        }
        assert(patternMapping[pattern] != -1);
        assert(patternMapping[rc_pattern] != -1);

    } else {
        // the pattern is known
        // check to see if the filename is also known
        fp_iter = filenameMapping.find(filename);
        if (fp_iter == filenameMapping.end() || filename != fp_iter->first) {
            // same pattern different filename
            // error in mapping file
            fprintf(stderr, "[WARNING]: The pattern \"%s\" is associated with both \"%s\" and \"%s\"\n",
                    pattern.c_str(),
                    filename.c_str(),
                    fp_iter->first.c_str());
        }
    }
}


void FileManager::add(std::string pattern) {
    std::map<std::string, int>::iterator fp_iter;

    // check if the pattern has been seen before
    std::map<std::string, int>::iterator pm_iter = patternMapping.find(pattern);

    if(pm_iter == patternMapping.end()) {
        // add in the pattern key to the hash
      std::string rc_pattern = pattern;
        reverseComplement(rc_pattern);

        patternMapping[pattern] = -1;
        patternMapping[rc_pattern] = -1;
        fp_iter = filenameMapping.find("");
        if (fp_iter == filenameMapping.end()) {
            // it hasn't so make a new filewrapper and associate the
            // pattern and the filename with it's index in the vector
            FileWrapper * fw = new FileWrapper("");
            // since the size value will always be one more than the index
            // after we push on the filewrapper below the index will match
            int n = files.size();
            patternMapping[pattern] = n;
            patternMapping[rc_pattern] = n;
            filenameMapping[""] = n;

            fw->filename = "";
            fw->closeAtEnd = false;
            fw->file = stdout;
            fw->fileOpened = true;
            files.push_back(fw);

        } else {
            // the file has been seen before but the pattern hasn't
            // so we need to associate the new pattern with the file
            patternMapping[pattern] = filenameMapping[""];
            patternMapping[rc_pattern] = filenameMapping[""];
        }
        assert(patternMapping[pattern] != -1);
        assert(patternMapping[rc_pattern] != -1);
    } else {
        // the pattern is known
        // check to see if the filename is also known
        fp_iter = filenameMapping.find("");
        if (fp_iter == filenameMapping.end() || "" != fp_iter->first) {
            // same pattern different filename
            // error in mapping file
            fprintf(stderr, "[WARNING]: The pattern \"%s\" is associated with both \"%s\" and \"%s\"\n",
                    pattern.c_str(),
                    "(stdout)",
                    fp_iter->first.c_str());
        }
    }
}

FILE * FileManager::find (std::string key) {
    FileWrapper * fw;
    std::map<std::string, int>::iterator iter = patternMapping.find(key);
    if(iter == patternMapping.end()) {
        //fprintf(stderr, "[ERROR]: The pattern \"%s\" is not found in the mapping\n", key.c_str());
        return NULL;
    }
    fw = files[iter->second];
    if(fw->fileOpened) {
        ++fw->recordsWritten;
        return fw->file;
    } else if (openCount >= OPEN_MAX) {
        // we're above the max number of allowed open files
        // pop the first one off the queue and close it
        FileWrapper * fo = closingQueue.top();
        fo->close();
        --openCount;
        closingQueue.pop();
    }
    int ret = fw->open();
    if(ret) {
        return NULL;
    }
    if(fw->file != stdout) {
        // sdtout doesn't really count as it is implicitly opened for the process
        // so we don't want to add it into our queue
        closingQueue.push(fw);
        ++openCount;
    }
    ++fw->recordsWritten;
    return fw->file;
}
