#! /bin/bash
#
# time_grep.sh
# Copyright (C) 2013 uqcskenn <uqcskenn@rudd>
#
# Distributed under terms of the MIT license.
#

impl=fgrep
for p in speed_test/random_100*
do
    patterns=$(basename $p)
    echo "real,user,sys,avemem,maxmem" >speed_test/${impl}_${patterns%.txt}_times.txt
    for i in {1..10}
    do
        /usr/bin/time -f "%E,%U,%S,%K,%M" -a -o speed_test/${impl}_${patterns%.txt}_times.txt $impl -f $p <(zcat $HOME/EBPR_phage/raw_data/M9248_s_5_CGATGT_sequence.txt.gz) | wc -l >>speed_test/${impl}_${patterns%.txt}_words.txt
    done
done


