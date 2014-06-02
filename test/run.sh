#!/bin/sh

diff_output_and_report() {
  diff $1 $2 >/dev/null
  if [ $? != 0 ]; then
      echo "FAILED test $3!"
  else
      echo "PASSED test $3!"
  fi

}

../fxtract CAAAGGGATTGAGACGCCACTT 1.fa > 1.output.fa
diff_output_and_report 1.output.fa 1.expected.fa 1a

../fxtract -G CAAAGGGATTGAGACGCCACTT 1.fa > 1.output.fa
diff_output_and_report 1.output.fa 1.expected.fa 1b

../fxtract -E CAAAGGGATTGAGACGCCACTT 1.fa > 1.output.fa
diff_output_and_report 1.output.fa 1.expected.fa 1c

../fxtract -P CAAAGGGATTGAGACGCCACTT 1.fa > 1.output.fa
diff_output_and_report 1.output.fa 1.expected.fa 1d
