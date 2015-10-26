#! /bin/sh

NAME=$1
shift
OUT=$1
shift

mkdir -p $OUT

./$NAME "$@" > $OUT/orig
./$NAME.slicm "$@" > $OUT/res

if diff -q $OUT/{orig,res}; then
    printf "[PASSED]"
else
    printf "[FAILED]"
fi
echo " $NAME"
