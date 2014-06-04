#!/bin/sh
set -x

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
