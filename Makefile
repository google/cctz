# Copyright 2016 Google Inc. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.

# While Bazel (http://bazel.io) is the primary build system used by cctz,
# this Makefile is provided as a convenience for those who can't use Bazel
# and can't compile the sources in their own build system.
#
# Suggested usage:
#
#   To build locally:
#     make -C build -f ../Makefile SRC=../ -j `nproc`
#
#   To build/install a versioned shared library on Linux:
#     make -C build -f ../Makefile SRC=../ -j `nproc` \
#         SHARED_LDFLAGS='-shared -Wl,-soname,libcctz.so.2' \
#         CCTZ_SHARED_LIB=libcctz.so.2.0 \
#         install install_shared_lib

# local configuration
CXX ?= g++
CXXFLAGS ?= -O3
STD ?= c++11
LDFLAGS ?= -O3
PREFIX ?= /usr/local
DESTDIR ?=

# possible support for googletest
## TESTS = civil_time_test time_zone_lookup_test time_zone_format_test
## TEST_FLAGS = ...
## TEST_LIBS = ...

VPATH = $(SRC)src:$(SRC)examples
CXXFLAGS += -g -Wall -I$(SRC)include -std=$(STD) \
            $(TEST_FLAGS) -fPIC -MMD
ARFLAGS = rcs
LINK.o = $(LINK.cc)
LDLIBS += $(TEST_LIBS)
SHARED_LDFLAGS = -shared
INSTALL = install

CCTZ = cctz
CCTZ_LIB = lib$(CCTZ).a
CCTZ_SHARED_LIB = lib$(CCTZ).so

CCTZ_HDRS = $(SRC)include/cctz/*.h

CCTZ_OBJS =			\
	civil_time_detail.o	\
	time_zone_fixed.o	\
	time_zone_format.o	\
	time_zone_if.o		\
	time_zone_impl.o	\
	time_zone_info.o	\
	time_zone_libc.o	\
	time_zone_lookup.o	\
	time_zone_posix.o       \
	zone_info_source.o

TOOLS = time_tool
EXAMPLES = classic epoch_shift hello example1 example2 example3 example4

all: $(TESTS) $(TOOLS) $(EXAMPLES)

$(TESTS) $(TOOLS) $(EXAMPLES): $(CCTZ_LIB)

$(TESTS:=.o) $(TOOLS:=.o) $(EXAMPLES:=.o):

$(CCTZ_LIB): $(CCTZ_OBJS)
	$(AR) $(ARFLAGS) $@ $(CCTZ_OBJS)

$(CCTZ_SHARED_LIB): $(CCTZ_OBJS)
	$(LINK.o) $(LDFLAGS) $(SHARED_LDFLAGS) -o $@ $(CCTZ_OBJS)

install: install_hdrs install_lib

install_hdrs: $(CCTZ_HDRS)
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/include/cctz
	$(INSTALL) -m 644 -p $? $(DESTDIR)$(PREFIX)/include/cctz

install_lib: $(CCTZ_LIB)
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/lib
	$(INSTALL) -m 644 -p $? $(DESTDIR)$(PREFIX)/lib

install_shared_lib: $(CCTZ_SHARED_LIB)
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/lib
	$(INSTALL) -m 644 -p $? $(DESTDIR)$(PREFIX)/lib

clean:
	@$(RM) $(EXAMPLES:=.dSYM) $(EXAMPLES:=.o) $(EXAMPLES:=.d) $(EXAMPLES)
	@$(RM) $(TOOLS:=.dSYM) $(TOOLS:=.o) $(TOOLS:=.d) $(TOOLS)
	@$(RM) $(TESTS:=.dSYM) $(TESTS:=.o) $(TESTS:=.d) $(TESTS)
	@$(RM) $(CCTZ_OBJS) $(CCTZ_OBJS:.o=.d) $(CCTZ_LIB) $(CCTZ_SHARED_LIB)

-include $(CCTZ_OBJS:.o=.d) $(TESTS:=.d) $(TOOLS:=.d) $(EXAMPLES:=.d)
