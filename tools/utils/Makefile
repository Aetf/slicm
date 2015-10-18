#
# Indicate where we are relative to the top of the source tree.
#
LEVEL=../..

#
# Give the name of the tool.
#
LIBRARYNAME=lamputils

#
# List libraries that we'll need
# We use LIBS because sample is a dynamic library.
#
#USEDLIBS = slicm

# Don't do -D__inline__= as this bones sys/stat.h
CFLAGS+=-DPRINT_CONFLICTS -DPRIEPSET -D_GNU_SOURCE -D_XOPEN_SOURCE=600 -Wall -pedantic -Wno-long-long -g -O0 -I.
CCFLAGS+=-D_XOPEN_SOURCE=600 -Wall -Wno-long-long -g -O1 -I.

#
# Include Makefile.common so we know what to do.
#
include $(LEVEL)/Makefile.common