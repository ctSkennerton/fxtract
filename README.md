fxtract
=======

Extract sequences from a fastx (fasta or fastq) file given a
subsequence. Currently uses a variety of search algorithms depending
on the task. Currently searches using a simple substring search for
basic tasks but can change to using POSIX regular expressions, PCRE,
hash lookups or multi-pattern searching as required. By default
will look in the sequence of each record but can be told to look
in the header, comment or quality sections of a record.

## Installation with pre-compiled binaries
Precompiled binaries are [available](https://github.com/ctSkennerton/fxtract/releases).

## Install from source
Installing from source requires a few shared libraries.
The boost iostream library is required and by proxy both [zlib](http://www.zlib.net/)
and [bzip2](http://www.bzip.org/)
libraries are required to be installed on your system; PCRE is an optional dependency.
If you do not have the boost iostream library installed (it does need to be
installed as it contains compiled components) follow the instructions on the
[Boost website](http://www.boost.org/doc/libs/1_55_0/libs/iostreams/doc/index.html).

To compile, make sure the dependencies above are installed and then
hopefully the following will work for you (if not open an
[issue](https://github.com/ctSkennerton/fxtract/issues))
```
$ git clone --recursive https://github.com/ctSkennerton/fxtract.git
$ cd fxtract
$ make
```
If PCRE is not installed on your system or if you get an error message during
installation try the following
```
$ make NO_PCRE=1
```
If you want to check to see if it's installed properly run the tests by
```
$ cd test
$ bash run.sh
```
and see if they pass, I hope that they do.


Usage
-----

```
$ fxtract  ATTACTAG /path/to/file.fq
$ fxtract -z ATTACTAG /path/to/file.fq.gz  # -z for gzip file
$ fxtract -j ATTACTAG /path/to/file.fq.bz2 # -j for bzip2 file
$ fxtract -j ATTACTAG /path/to/file.fq.bz2 /path/to/file2.fq.bz2  # paired reads
$ fxtract -H seq_1 /path/to/file.fa  # search for headers
```
