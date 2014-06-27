#!/bin/sh

diff_output_and_report() {
  diff $1 $2 >/dev/null
  if [ $? != 0 ]; then
      printf "\x1b[31m%s\x1b[0m\n" "FAILED test $3!"
  else
      printf "PASSED test $3!\n"
  fi

}

# search for sequences using the different regex engines
../fxtract CAAAGGGATTGAGACGCCACTT 1.fa > 1.output.fa
diff_output_and_report 1.output.fa 1.expected.fa 1a

../fxtract -G CAAAGGGATTGAGACGCCACTT 1.fa > 1.output.fa
diff_output_and_report 1.output.fa 1.expected.fa 1b

../fxtract -E CAAAGGGATTGAGACGCCACTT 1.fa > 1.output.fa
diff_output_and_report 1.output.fa 1.expected.fa 1c

../fxtract -P CAAAGGGATTGAGACGCCACTT 1.fa > 1.output.fa
diff_output_and_report 1.output.fa 1.expected.fa 1d

# search for headers
../fxtract -H "HISEQ2000:55:C0JRTACXX:2:1101:11128:12710_1:N:0:CTTGTAAT" 1.fa > 2.output.fa
diff_output_and_report 2.output.fa 2.expected.fa 2a
../fxtract -HG "HISEQ2000:55:C0JRTACXX:2:1101:11128:12710_1:N:0:CTTGTAAT" 1.fa > 2.output.fa
diff_output_and_report 2.output.fa 2.expected.fa 2b
../fxtract -HE "HISEQ2000:55:C0JRTACXX:2:1101:11128:12710_1:N:0:CTTGTAAT" 1.fa > 2.output.fa
diff_output_and_report 2.output.fa 2.expected.fa 2c
../fxtract -HP "HISEQ2000:55:C0JRTACXX:2:1101:11128:12710_1:N:0:CTTGTAAT" 1.fa > 2.output.fa
diff_output_and_report 2.output.fa 2.expected.fa 2d
../fxtract -HX "HISEQ2000:55:C0JRTACXX:2:1101:11128:12710_1:N:0:CTTGTAAT" 1.fa > 2.output.fa
diff_output_and_report 2.output.fa 2.expected.fa 2e


# multipattern search
../fxtract -Hf headers.txt 1.fa > 3.output.fa
diff_output_and_report 3.output.fa 3.expected.fa 3

# paired reads
../fxtract -H "HSQ868392H08B7ADXX:2:1112:8977:35114" 4_1.fa 4_2.fa > 4.output.fa
diff_output_and_report 4.output.fa 4.expected.fa 4

# search for comment strings
../fxtract -C "Accumulibacter" 5.fa > 5.output.fa
diff_output_and_report 5.output.fa 5.expected.fa 5


# inverse the match
../fxtract -Hv HISEQ2001 1.fa > 6.output.fa
diff_output_and_report 6.output.fa 6.expected.fa 6

# count the matches
../fxtract -Hc HISEQ2000 1.fa > 7.output.fa
diff_output_and_report 7.output.fa 7.expected.fa 7

# test out different fasta file styles
../fxtract -H 1101:11128:12710 8.fa >8.output.fa
diff_output_and_report 8.output.fa 8.expected.fa 8

../fxtract -H 1101:11128:12710 9.fa >9.output.fa
diff_output_and_report 9.output.fa 8.expected.fa 9

# compressed files
gzip 8.fa
../fxtract -H 1101:11128:12710 8.fa.gz >8.output.fa
diff_output_and_report 8.output.fa 8.expected.fa 10a

gunzip 8.fa.gz
bzip2 8.fa
../fxtract -H 1101:11128:12710 8.fa.bz2 >8.output.fa
diff_output_and_report 8.output.fa 8.expected.fa 10a
bunzip2 8.fa.bz2

gzip 4_1.fa 4_2.fa
../fxtract -H "HSQ868392H08B7ADXX:2:1112:8977:35114" 4_1.fa.gz 4_2.fa.gz > 4.output.fa
diff_output_and_report 4.output.fa 4.expected.fa 10c
gunzip 4_1.fa.gz 4_2.fa.gz

bzip2 4_1.fa 4_2.fa
../fxtract -H "HSQ868392H08B7ADXX:2:1112:8977:35114" 4_1.fa.bz2 4_2.fa.bz2 > 4.output.fa
diff_output_and_report 4.output.fa 4.expected.fa 10d
bunzip2 4_1.fa.bz2 4_2.fa.bz2


