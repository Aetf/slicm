THIS_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
PASSLIB = $(THIS_DIR)../build/Debug+Asserts/lib/slicm.so
LAMPHOOK = $(THIS_DIR)../build/Debug+Asserts/lib/liblamphooks.a ../build/Debug+Asserts/lib/liblamputils.a

RESULTDIR = $(THIS_DIR)results
OUTPUTDIR = $(THIS_DIR)output

CASES = case1 case2 case3 case4

all : $(CASES)

case1 : correct1.bc

cfg1 : correct1.ll correct1.bc
	$(eval TMP := $(shell opt -view-cfg correct1.bc 2>&1 >/dev/null | sed -rn 's#^.*erase graph file: (/tmp/cfg.*-[0-9a-zA-Z]+\.dot)$$$$#\1#p'))
	sleep 1s
	rm $(TMP)

.PHONY : all $(CASES)

%.ll : %.bc
	llvm-dis -o $@ $<

%.bc : %.c
	clang -emit-llvm -c -o $@ $<

%.slicm.bc : %.bc
	opt -stats -load $(PASSLIB) -slicm -o $@ $<
