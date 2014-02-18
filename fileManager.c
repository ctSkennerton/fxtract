/*
 * fileManager.cpp
 * Copyright (C) 2013 uqcskenn <uqcskenn@hawke>
 *
 * Distributed under terms of the MIT license.
 */

#include <sys/syslimits.h>
#include "fileManager.h"

FileWrapper * filewrapper_new() {
    FileWrapper * fw = (FileWrapper *) malloc(sizeof(FileWrapper));
    fw->file = NULL;
    fw->filename = NULL;
    fw->fileOpened = false;
    fw->closeAtEnd = false;
    fw->recordsWritten = 0;
}

FileWrapper * filewrapper_new2(sds filename) {
    FileWrapper * fw = (FileWrapper *) malloc(sizeof(FileWrapper));
    fw->file = NULL;
    fw->filename = filename;
    fw->fileOpened = false;
    fw->closeAtEnd = false;
    fw->recordsWritten = 0;
}
    
int filewrapper_open(FileWrapper * fw) {
    if(!fw->fileOpened) {
        fw->file = fopen(fw->filename, "a");
    }
    
    if (fw->file == NULL) {
        perror("problem opening file");
        return 1;
    }
    fw->fileOpened = true;
    return 0;
}

void filewrapper_close(FileWrapper * fw) {
    if(fw->fileOpened & fw->closeAtEnd) {
        fclose(fw->file);
        fw->fileOpened = false;
    }
}

void filewrapper_delete(FileWrapper * fw) {
    filewrapper_close(fw);
    sdsfree(fw->filename);
    free(fw);
}

FileManager * filemanager_new() {
    FileManager * fm = (FileManager *) malloc(sizeof(FileManager));
    fm->openCount = 0;
    fm->queue = create_priority_queue(OPEN_MAX, &filemanager_pqcomp);
    fm->patternMapping = kh_init(s2d);
    kv_init(fm->files);
    return fm;
}

void filemanager_delete(FileManager * fm) {
    khiter_t k;
    for (k = kh_begin(fm->patternMapping); k != kh_end(fm->patternMapping); ++k) {
		if (kh_exist(fm->patternMapping, k)){ 
            sdsfree(kh_key(fm->patternMapping, k));
            int file_index  = kh_value(fm->patternMapping, k);
            filewrapper_delete(kv_A(fm->files, file_index));
        }
    }
    /*for (k = kh_begin(fm->filenameMapping); k != kh_end(fm->filenameMapping); ++k) {
		if (kh_exist(fm->filenameMapping, k)){ 
            sdsfree(kh_key(fm->filenameMapping, k));

            // this shouldn't be needed since we delete them above

            //int file_index  = kh_value(fm->filenameMapping, k);
            //filewrapper_delete(kv_A(fm->files, file_index));
        }
    }*/
    kv_destroy(fm->files);
    kh_destroy(s2d, fm->patternMapping);
    kh_destroy(s2d, fm->filenameMapping);
    free_priority_queue(&fm->queue);
}

int filemanager_pqcomp(const void * a, const void * b) {
    const FileWrapper * A = (FileWrapper *) a;
    const FileWrapper * B = (FileWrapper *) b;
    /* Most priority queues give you back the element with the highest value
     * but here we want the smallest values (ie files used least often) so 
     * less than (<) actually returns GREATER
     */
    if(A->recordsWritten < B->recordsWritten)
        return GREATER;
    else if(A->recordsWritten > B->recordsWritten)
        return SMALLER;
    else
        return EQUAL;
}

void filemanager_add2(FileManager * fm, sds pattern, sds filename) {
    khiter_t k_fm, k_pm;
    int ret;
    // check if the pattern has been seen before
    k_pm = kh_get(s2d, fm->patternMapping, pattern);
    if(k_pm == kh_end(fm->patternMapping)) {
        
        // add in the pattern key to the hash
        k_pm = kh_put(s2d, fm->patternMapping, sdsdup(pattern), &ret);
        
        // now check to see if the filename has been seen before
        k_fm = kh_get(s2d, fm->filenameMapping, filename);
        if (k_fm == kh_end(fm->filenameMapping)) {
            // it hasn't so make a new filewrapper and associate the
            // pattern and the filename with it's index in the vector
            sds stored_name = sdsdup(filename);

            FileWrapper * fw = filewrapper_new2(stored_name);
            // since the size value will always be one more than the index
            // after we push on the filewrapper below the index will match
            int n = kv_size(fm->files);
            kh_value(fm->patternMapping, k_pm) = n;

            k_fm = kh_put(s2d, fm->filenameMapping, stored_name, &ret);
            kh_value(fm->filenameMapping, k_fm) = n;

            fw->filename = stored_name;
            fw->closeAtEnd = true;
            kv_push(FileWrapper *, fm->files, fw);

        } else {
            // the file has been seen before but the pattern hasn't
            // so we need to associate the new pattern with the file
            kh_value(fm->patternMapping, k_pm) = kh_value(fm->filenameMapping, k_fm);
        }   
    } else {
        // the pattern is known
        // check to see if the filename is also known
        k_fm = kh_get(s2d, fm->filenameMapping, filename);
        if (k_fm == kh_end(fm->filenameMapping) || !strcmp(filename, kh_key(fm->filenameMapping, k_fm))) {
            // same pattern different filename
            // error in mapping file
            fprintf(stderr, "[WARNING]: The pattern \"%s\" is associated with both \"%s\" and \"%s\"\n", 
                    pattern, 
                    filename, 
                    kh_key(fm->filenameMapping, k_fm));
        }
    }
}

void filemanager_add(FileManager * fm, sds pattern) {
    khiter_t k_pm, k_fm;
    int ret;
    // check if the pattern has been seen before
    k_pm = kh_get(s2d, fm->patternMapping, pattern);
    if(k_pm == kh_end(fm->patternMapping)) {
        // add in the pattern key to the hash
        k_pm = kh_put(s2d, fm->patternMapping, sdsdup(pattern), &ret);

        k_fm = kh_get(s2d, fm->filenameMapping, "");
        if (k_fm == kh_end(fm->filenameMapping)) {
            // it hasn't so make a new filewrapper and associate the
            // pattern and the filename with it's index in the vector
            sds stored_name = sdsempty();
            FileWrapper * fw = filewrapper_new2(stored_name);
            // since the size value will always be one more than the index
            // after we push on the filewrapper below the index will match
            int n = kv_size(fm->files);
            kh_value(fm->patternMapping, k_pm) = n;

            k_fm = kh_put(s2d, fm->filenameMapping, stored_name, &ret);
            kh_value(fm->filenameMapping, k_fm) = n;

            fw->filename = stored_name;
            fw->closeAtEnd = false;
            fw->file = stdout;
            fw->fileOpened = true;
            kv_push(FileWrapper *, fm->files, fw);

        } else {
            // the file has been seen before but the pattern hasn't
            // so we need to associate the new pattern with the file
            kh_value(fm->patternMapping, k_pm) = kh_value(fm->filenameMapping, k_fm);
        }   
    } else {
        // the pattern is known
        // check to see if the filename is also known
        k_fm = kh_get(s2d, fm->filenameMapping, "");
        if (k_fm == kh_end(fm->filenameMapping) || !strcmp("", kh_key(fm->filenameMapping, k_fm))) {
            // same pattern different filename
            // error in mapping file
            fprintf(stderr, "[WARNING]: The pattern \"%s\" is associated with both \"%s\" and \"%s\"\n", 
                    pattern, 
                    "(stdout)", 
                    kh_key(fm->filenameMapping, k_fm));
        }
    }
}

FILE * filemanager_find (FileManager * fm, sds key) {
    FileWrapper * fw;
    khiter_t k = kh_get(s2d, fm->patternMapping, key);
    if(k == kh_end(fm->patternMapping)) {
        fprintf(stderr, "[ERROR]: The pattern \"%s\" is not found in the mapping\n", key);
        return NULL;
    }
    fw = kv_A(fm->files, kh_value(fm->patternMapping, k));
    if(fw->fileOpened) {
        ++fw->recordsWritten;
        return fw->file;
    } else if (fm->openCount >= OPEN_MAX) {
        // we're above the max number of allowed open files
        // pop the first one off the queue and close it
        FileWrapper * fo = (FileWrapper *) priority_queue_poll(fm->queue);
        filewrapper_close(fo);
        --fm->openCount;
    }
    int ret = filewrapper_open(fw);
    if(ret) {
        return NULL;
    }
    if(fw->file != stdout) {
        // sdtout doesn't really count as it is implicitly opened for the process
        // so we don't want to add it into our queue
        priority_queue_insert(fm->queue, fw);
        ++fm->openCount;
    }
    ++fw->recordsWritten;
    return fw->file;
}
