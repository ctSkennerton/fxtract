#!/bin/sh
../fxtract/fxtract CAAAGGGATTGAGACGCCACTT 1.fa > 1.output.fa
diff 1.output.fa 1.expected.fa >/dev/null
if [ $? != 0 ]; then
    echo "FAILED test 1!"
else
    echo "PASSED test1!"
fi
