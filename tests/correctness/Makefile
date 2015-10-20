THIS_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
LEVEL := "../.."
PASSLIB = $(THIS_DIR)$(LEVEL)/build/Release+Asserts/lib/slicm.so

RESULTDIR = $(THIS_DIR)results
OUTPUTDIR = $(THIS_DIR)output

CASES = case1 case2 case3 case4

all : $(CASES)

case1 : correct1.bc

cfg1 : correct1.bc
	$(eval $@_TMP := $(shell opt -view-cfg $< 2>&1 >/dev/null | sed -rn 's#^.*erase graph file: (/tmp/cfg.*-[0-9a-zA-Z]+\.dot)$$$$#\1#p'))
	@sleep 1s
	@rm $($@_TMP)

slicmcfg1 : correct1.slicm.bc
	$(eval $@_TMP := $(shell opt -view-cfg $< 2>&1 >/dev/null | sed -rn 's#^.*erase graph file: (/tmp/cfg.*-[0-9a-zA-Z]+\.dot)$$$$#\1#p'))
	@sleep 1s
	@rm $($@_TMP)

.PHONY : all $(CASES) cfg1

%.ll : %.bc
	llvm-dis -o $@ $<

%.bc : %.c
	clang -emit-llvm -c -o $@ $<

%.slicm.bc : %.bc
	opt -stats -load $(PASSLIB) -slicm -o $@ $<
