#!/usr/bin/env python
from __future__ import print_function
import sys, os, glob, re

fixed_re = re.compile('fxtract-(\w+)_random_(10{2,5})_(\d\d)bp_patterns_times\.txt')
norm_re = re.compile('fxtract-(\w+)_random_(10{2,5})_patterns_minus_small_times\.txt')

impl_codes = {'ac': 'aho-corasick', 'sh': 'set horspool', 'mb': 'multi-bfam',
'wm': 'wu-manber', 'cwm': 'non-seqan wu-manber'}

times_files = glob.glob("./fxtract*times.txt")
data = {}
for fn in times_files:
    
    match = fixed_re.search(fn)
    if match:
        impl = impl_codes[match.group(1)]
        pattern_type = match.group(3)
        pattern_count = match.group(2)
    else:
        match = norm_re.search(fn)
        if match:
            impl = impl_codes[match.group(1)]
            pattern_type = 'norm'
            pattern_count = match.group(2)
        else:
            raise ValueError("Filename %s does not fit either form" % fn)
    with open(fn) as fp:
        first_line = True
        seconds = 0.0
        memory = 0.0
        for line in fp:
            if first_line:
                first_line = False
                continue
            fields = line.rstrip().split(',')
            seconds += float(fields[1]) + float(fields[2])
            memory += float(fields[-1])

        seconds /= 10.0
        memory /= 10.0
        print(impl,pattern_type, pattern_count, seconds, memory, sep="\t")
