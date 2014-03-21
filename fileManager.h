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

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "khash.h"
#include "kvec.h"
#include "pq.h"
#include "sds/sds.h"

KHASH_INIT(s2d, sds, int, 1, kh_str_hash_func, kh_str_hash_equal);

typedef struct _FileWrapper {
    FILE *                file;           // our file
    sds                   filename;       // the filename that we are writting to
    bool                  fileOpened;     // is our file opened?
    bool                  closeAtEnd;     // should this FILE be closed when finished - not to be used for stdout, stderr
    size_t                recordsWritten; // count of fasta/fastq records written to this file - used in priority queue comparison
} FileWrapper;


typedef struct _FileManager {
    int                   openCount;       // keep count of the number of files that we've opened
    khash_t(s2d) *        patternMapping;  // hash of patterns to indexes into the array of filewrappers
    khash_t(s2d) *        filenameMapping; // hash of filenames to indexes into the array of filewrappers
    kvec_t(FileWrapper *) files;           // An array of all of the file wrappers that are being managed
    priority_queue *      queue;           // priority queue used to determine which filewrapper should be closed first when there are too many
} FileManager;


/* Allocate memory for a new FileWrapper and initalise values
 */
FileWrapper *  filewrapper_new    ();
/* Same as above but sets the filename field to the function argument
 */
FileWrapper *  filewrapper_new2   (sds filename);
/* Deallocate memory
 */
void          filewrapper_delete (FileWrapper * fw);
/* Open a stream in append mode and write
 * sets the fileOpened field to true
 * returns 1 if there was a problem or 0 on sucess
 */
int           filewrapper_open   (FileWrapper * fw);
/* Closes the associated file stream if it is opened and the closeAtEnd
 * field is false.  sets fileOpened field to false if the file is closed
 */
void          filewrapper_close  (FileWrapper * fw);


FileManager * filemanager_new    ();
void          filemanager_delete (FileManager * fm);
void          filemanager_add    (FileManager * fm, sds key);
void          filemanager_add2   (FileManager * fm, sds key, sds fileName);
int           filemanager_pqcomp (const void  * a, const void * b);
FILE *        filemanager_find   (FileManager * fm, sds key);
int           filemanager_npat   (FileManager * fm);


#endif /* !__FILEMANAGER_H__ */

