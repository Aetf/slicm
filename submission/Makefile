DIR=peifeng_hw2

all: submit

submit: $(DIR).tgz
	scp $(DIR).tgz peifeng@eecs583a.eecs.umich.edu:/hw2_submissions/

$(DIR).tgz: $(DIR)/README src results
	tar -cvzf $(DIR).tgz $(DIR)

src:
	mkdir -p $(DIR)/src
	cp -r ../passes/*  $(DIR)/src
	rm -f $(DIR)/src/Makefile

results: correct perf

correct: ../tests/correctness/Makefile
	mkdir -p $(DIR)/results
	DEBUG=0 make -C ../tests/correctness -s clean all
	cp ../tests/correctness/*.slicm.bc $(DIR)/results

perf: ../tests/performance/Makefile
	mkdir -p $(DIR)/results
	DEBUG=0 make -C ../tests/performance -s clean all
	cp ../tests/performance/*.slicm.bc $(DIR)/results
	cp ../tests/performance/*.intelligent-slicm.bc $(DIR)/results

.PHONY: all submit src results correct perf
