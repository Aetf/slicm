#! /bin/sh

mkdir -p $2

./$1 > $2/orig
./$1.slicm > $2/orig
if diff -q $2/{orig,res}; then
    echo "PASSED"
else
    echo "FAILED"
fi
