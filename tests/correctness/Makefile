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

CASES = case1 case2 case3 case4 case5

# Tools we use
LLVMHOME = /opt/llvm33
opt = $(LLVMHOME)/bin/opt
clang = $(LLVMHOME)/bin/clang
llvm-dis = $(LLVMHOME)/bin/llvm-dis

all : $(CASES)

case1 : correct1 correct1.slicm
	./check.sh $< $(OUTPUTDIR)

case2 : correct2 correct2.slicm
	./check.sh $< $(OUTPUTDIR)

case3 : correct3 correct3.slicm
	./check.sh $< $(OUTPUTDIR)

case4 : correct4 correct4.slicm
	./check.sh $< $(OUTPUTDIR)

case5 : correct5 correct5.slicm
	./check.sh $< $(OUTPUTDIR)

cfg1 : correct1.bc
	$(eval $@_TMP := $(shell opt -view-cfg $< 2>&1 >/dev/null | sed -rn 's#^.*erase graph file: (/tmp/cfg.*-[0-9a-zA-Z]+\.dot)$$$$#\1#p'))
	@sleep 1s
	@rm $($@_TMP)

slicmcfg1 : correct1.slicm.bc
	$(eval $@_TMP := $(shell opt -view-cfg $< 2>&1 >/dev/null | sed -rn 's#^.*erase graph file: (/tmp/cfg.*-[0-9a-zA-Z]+\.dot)$$$$#\1#p'))
	@sleep 1s
	@rm $($@_TMP)

cfg2 : correct2.bc
	$(eval $@_TMP := $(shell opt -view-cfg $< 2>&1 >/dev/null | sed -rn 's#^.*erase graph file: (/tmp/cfg.*-[0-9a-zA-Z]+\.dot)$$$$#\1#p'))
	@sleep 1s
	@rm $($@_TMP)

slicmcfg2 : correct2.slicm.bc
	$(eval $@_TMP := $(shell opt -view-cfg $< 2>&1 >/dev/null | sed -rn 's#^.*erase graph file: (/tmp/cfg.*-[0-9a-zA-Z]+\.dot)$$$$#\1#p'))
	@sleep 1s
	@rm $($@_TMP)

cfg3 : correct3.bc
	$(eval $@_TMP := $(shell opt -view-cfg $< 2>&1 >/dev/null | sed -rn 's#^.*erase graph file: (/tmp/cfg.*-[0-9a-zA-Z]+\.dot)$$$$#\1#p'))
	@sleep 1s
	@rm $($@_TMP)

slicmcfg3 : correct3.slicm.bc
	$(eval $@_TMP := $(shell opt -view-cfg $< 2>&1 >/dev/null | sed -rn 's#^.*erase graph file: (/tmp/cfg.*-[0-9a-zA-Z]+\.dot)$$$$#\1#p'))
	@sleep 1s
	@rm $($@_TMP)

cfg4 : correct4.bc
	$(eval $@_TMP := $(shell opt -view-cfg $< 2>&1 >/dev/null | sed -rn 's#^.*erase graph file: (/tmp/cfg.*-[0-9a-zA-Z]+\.dot)$$$$#\1#p'))
	@sleep 1s
	@rm $($@_TMP)

slicmcfg4 : correct4.slicm.bc
	$(eval $@_TMP := $(shell opt -view-cfg $< 2>&1 >/dev/null | sed -rn 's#^.*erase graph file: (/tmp/cfg.*-[0-9a-zA-Z]+\.dot)$$$$#\1#p'))
	@sleep 1s
	@rm $($@_TMP)

cfg5 : correct5.bc
	$(eval $@_TMP := $(shell opt -view-cfg $< 2>&1 >/dev/null | sed -rn 's#^.*erase graph file: (/tmp/cfg.*-[0-9a-zA-Z]+\.dot)$$$$#\1#p'))
	@sleep 1s
	@rm $($@_TMP)

slicmcfg5 : correct5.slicm.bc
	$(eval $@_TMP := $(shell opt -view-cfg $< 2>&1 >/dev/null | sed -rn 's#^.*erase graph file: (/tmp/cfg.*-[0-9a-zA-Z]+\.dot)$$$$#\1#p'))
	@sleep 1s
	@rm $($@_TMP)

slicm1 : correct1.slicm.bc

slicm2 : correct2.slicm.bc

slicm3 : correct3.slicm.bc

slicm4 : correct4.slicm.bc

slicm5 : correct5.slicm.bc

clean :
	rm -f *.bc *.ll
	rm -f correct{1,2,3,4,5}
	rm -f correct{1,2,3,4,5}.slicm

.PHONY : all clean $(CASES) cfg1 cfg2 cfg3 cfg4 cfg5 slicmcfg1 slicmcfg2 slicmcfg3 slicmcfg4 slicmcfg5

%.ll : %.bc
	$(llvm-dis) -o $@ $<

%.bc : %.c
	$(clang) -emit-llvm -c -o $@ $<

%.slicm.bc : %.bc
	$(opt) $(DEBUG_FLAG) -load $(PASSLIB) -slicm -o $@ $<

correct% : correct%.bc
	$(clang) -o $@ $<

correct%.slicm : correct%.slicm.bc
	$(clang) -o $@ $<
