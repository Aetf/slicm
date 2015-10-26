THIS_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
LEVEL := "../.."
RELPASSLIB = $(LEVEL)/build/Debug+Asserts/lib/slicm.so
PASSLIB = $(THIS_DIR)$(RELPASSLIB)

DEBUG ?= 1
ifeq ($(DEBUG),1)
DEBUG_FLAG = -debug -stats
else
DEBUG_FLAG =
endif

RESULTDIR = $(THIS_DIR)results
OUTPUTDIR = $(THIS_DIR)output

CASES = case1 case2 case3 casewc

# Tools we use
LLVMHOME = /opt/llvm33
opt = $(LLVMHOME)/bin/opt
clang = $(LLVMHOME)/bin/clang
llvm-dis = $(LLVMHOME)/bin/llvm-dis

all : $(CASES)

case1 : perf1 perf1.slicm perf1.intelligent-slicm.bc
	./check.sh $< $(OUTPUTDIR)

case2 : perf2 perf2.slicm perf2.intelligent-slicm.bc
	./check.sh $< $(OUTPUTDIR)

case3 : perf3 perf3.slicm perf3.intelligent-slicm.bc
	./check.sh $< $(OUTPUTDIR)

casewc : wc wc.slicm 583wc.intelligent-slicm.bc
	./check.sh $< $(OUTPUTDIR) input/cccp.c

cfg1 : perf1.bc
	$(eval $@_TMP := $(shell opt -view-cfg $< 2>&1 >/dev/null | sed -rn 's#^.*erase graph file: (/tmp/cfg.*-[0-9a-zA-Z]+\.dot)$$$$#\1#p'))
	@sleep 1s
	@rm $($@_TMP)

slicmcfg1 : slicm1
	$(eval $@_TMP := $(shell opt -view-cfg $< 2>&1 >/dev/null | sed -rn 's#^.*erase graph file: (/tmp/cfg.*-[0-9a-zA-Z]+\.dot)$$$$#\1#p'))
	@sleep 1s
	@rm $($@_TMP)

cfg2 : perf2.bc
	$(eval $@_TMP := $(shell opt -view-cfg $< 2>&1 >/dev/null | sed -rn 's#^.*erase graph file: (/tmp/cfg.*-[0-9a-zA-Z]+\.dot)$$$$#\1#p'))
	@sleep 1s
	@rm $($@_TMP)

slicmcfg2 : slicm2
	$(eval $@_TMP := $(shell opt -view-cfg $< 2>&1 >/dev/null | sed -rn 's#^.*erase graph file: (/tmp/cfg.*-[0-9a-zA-Z]+\.dot)$$$$#\1#p'))
	@sleep 1s
	@rm $($@_TMP)

cfg3 : perf3.bc
	$(eval $@_TMP := $(shell opt -view-cfg $< 2>&1 >/dev/null | sed -rn 's#^.*erase graph file: (/tmp/cfg.*-[0-9a-zA-Z]+\.dot)$$$$#\1#p'))
	@sleep 1s
	@rm $($@_TMP)

slicmcfg3 : slicm3
	$(eval $@_TMP := $(shell opt -view-cfg $< 2>&1 >/dev/null | sed -rn 's#^.*erase graph file: (/tmp/cfg.*-[0-9a-zA-Z]+\.dot)$$$$#\1#p'))
	@sleep 1s
	@rm $($@_TMP)

cfgwc : wc.bc
	$(eval $@_TMP := $(shell opt -view-cfg $< 2>&1 >/dev/null | sed -rn 's#^.*erase graph file: (/tmp/cfg.*-[0-9a-zA-Z]+\.dot)$$$$#\1#p'))
	@sleep 1s
	@rm $($@_TMP)

slicmcfgwc : slicmwc
	$(eval $@_TMP := $(shell opt -view-cfg $< 2>&1 >/dev/null | sed -rn 's#^.*erase graph file: (/tmp/cfg.*-[0-9a-zA-Z]+\.dot)$$$$#\1#p'))
	@sleep 1s
	@rm $($@_TMP)

slicm1 : perf1.slicm.bc

slicm2 : perf2.slicm.bc

slicm3 : perf3.slicm.bc

slicmwc : perfwc.slicm.bc

clean :
	rm -f *.bc *.ll
	rm -f perf{1,2,3} wc
	rm -f perf{1,2,3}.slicm wc.slicm

.PHONY : all clean $(CASES) cfg1 cfg2 cfg3 cfgwc slicmcfg1 slicmcfg2 slicmcfg3 slicmcfgwc

%.ll : %.bc
	$(llvm-dis) -o $@ $<

%.bc : %.c
	$(clang) -emit-llvm -c -o $@ $<

%.slicm.bc : %.bc
	$(opt) $(DEBUG_FLAG) -load $(PASSLIB) -slicm -o $@ $<

%.intelligent-slicm.bc : %.slicm.bc
	cp $< $@

perf% : perf%.bc
	$(clang) -o $@ $<

wc : 583wc.bc
	$(clang) -o $@ $<

perf%.slicm : perf%.slicm.bc
	$(clang) -o $@ $<

wc.slicm : 583wc.slicm.bc
	$(clang) -o $@ $<
