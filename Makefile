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

CC = $(CXX)
OPT = -g
# TEST_FLAGS =
# TEST_LIBS =
CPPFLAGS = -Isrc $(TEST_FLAGS) -Wall -std=c++11 -pthread $(OPT) -fPIC
VPATH = examples:src
LDFLAGS = -pthread
LDLIBS = $(TEST_LIBS) -lm
ARFLAGS = rcs
PREFIX = /usr/local

CCTZ_LIB = libcctz.a

CCTZ_HDRS =			\
	civil_time.h		\
	civil_time_detail.h	\
	time_zone.h

CCTZ_OBJS =			\
	time_zone_format.o	\
	time_zone_if.o		\
	time_zone_impl.o	\
	time_zone_info.o	\
	time_zone_libc.o	\
	time_zone_lookup.o	\
	time_zone_posix.o

# TESTS = civil_time_test time_zone_lookup_test time_zone_format_test
TOOLS = time_tool
EXAMPLES = classic epoch_shift hello example1 example2 example3 example4

all: $(TESTS) $(TOOLS) $(EXAMPLES)

$(TESTS) $(TOOLS) $(EXAMPLES): $(CCTZ_LIB)

$(CCTZ_LIB): $(CCTZ_OBJS)
	$(AR) $(ARFLAGS) $@ $(CCTZ_OBJS)

install: $(CCTZ_HDRS) $(CCTZ_LIB)
	sudo cp -p $(CCTZ_HDRS) $(PREFIX)/include
	sudo cp -p $(CCTZ_LIB) $(PREFIX)/lib

clean:
	@$(RM) $(EXAMPLES:=.o) $(EXAMPLES)
	@$(RM) $(TOOLS:=.o) $(TOOLS)
	@$(RM) $(TESTS:=.o) $(TESTS)
	@$(RM) $(CCTZ_OBJS) $(CCTZ_LIB)

# dependencies

time_zone_format.o: time_zone.h civil_time.h time_zone_if.h
time_zone_if.o: time_zone_if.h time_zone.h civil_time.h \
 time_zone_info.h time_zone_libc.h tzfile.h
time_zone_impl.o: time_zone_impl.h time_zone.h civil_time.h \
 time_zone_info.h time_zone_if.h tzfile.h
time_zone_info.o: time_zone_info.h time_zone.h civil_time.h \
 time_zone_posix.h time_zone_if.h tzfile.h
time_zone_libc.o: time_zone_libc.h time_zone.h civil_time.h \
  time_zone_if.h
time_zone_lookup.o: time_zone.h civil_time.h \
  time_zone_impl.h time_zone_info.h time_zone_if.h tzfile.h
time_zone_posix.o: time_zone_posix.h

civil_time_test.o: civil_time.h
time_zone_lookup_test.o: time_zone.h civil_time.h
time_zone_format_test.o: time_zone.h civil_time.h

time_tool.o: time_zone.h civil_time.h

hello.o: time_zone.h civil_time.h
example1.o: time_zone.h civil_time.h
example2.o: time_zone.h civil_time.h
example3.o: time_zone.h civil_time.h
example4.o: time_zone.h civil_time.h

civil_time.h: civil_time_detail.h
