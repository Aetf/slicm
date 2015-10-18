#
# Indicate where we are relative to the top of the source tree.
#
LEVEL=../..

#
# Give the name of the tool.
#
LIBRARYNAME=lamphooks

#
# List libraries that we'll need, only for static libraries,
# for dynamic, use LIBS
#
USEDLIBS = lamputils

# Don't do -D__inline__= as this bones sys/stat.h
CPPFLAGS+=-D_GNU_SOURCE -D_XOPEN_SOURCE=600 -c -Wall -pedantic -Wno-long-long -g -O0 -I. -std=c++0x

#
# Include Makefile.common so we know what to do.
#
include $(LEVEL)/Makefile.common