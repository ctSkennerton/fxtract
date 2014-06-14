fxtract
=======

Extract sequences from a fastx (fasta or fastq) file given a
subsequence. Currently uses a variety of search algorithms depending
on the task. Currently searches using a simple substring search for
basic tasks but can change to using POSIX regular expressions, PCRE,
hash lookups or multi-pattern searching as required. By default
will look in the sequence of each record but can be told to look
in the header, comment or quality sections of a record.


Install
-------
Hopefully the following will work for you, if not open an
[issue](https://github.com/ctSkennerton/fxtract/issues)
```
$ git clone --recursive https://github.com/ctSkennerton/fxtract.git
$ cd fxtract
$ make
```
The only external library dependancy is PCRE, if you don't have it on
your system or if you get an error message during instalation try the
following
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
Currently fxtract does not support compressed files. If you want this
functionality you can easily add it using bash and anonymous named pipes
```
$ fxtract ATTACTAG <(gunzip -c /path/to/file.gz)
```
this mechanism is generalizable to multiple input files with any
compression algorithm
```
$ fxtract ATTACTAG <(bunzip2 -c /path/to/file.bz2) # bzip2
$ fxtract ATTACTAG <(bunzip2 -c /path/to/file.bz2) <(bunzip2 -c /path/to/file2.bz2) # two files
$ fxtract ATTACTAG <(unxz -c /path/to/file.xz) # xz
```
