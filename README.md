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
The only external dependancy is [zlib](http://www.zlib.net/), PCRE
is an optional dependency. Both of these are oftne already installed

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
$ make test_fxtract
```
and see if they pass, I hope that they do.

