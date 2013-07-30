fxtract
=======

Extract sequences from a fastx file given a subsequence


install
-------
When calling make you must set the `SEQAN` variable on the commandline.
This variable must equal the compiler flag passing through the path
to the seqan libray

```
make SEQAN=-I<path_to_seqan>
```

if your system has either zlib or bzip2 installed also set the 
`ZLIB` or `BZIP2` values to 1

```
make SEQAN=-I<path_to_seqan> ZLIB=1 BZIP2=1
```
