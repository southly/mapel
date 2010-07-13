# Copyright (c) 2010 NANRI <southly@gmail.com>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

DEBUG =

BIN = ./mapel
CC = gcc
CXX = g++
INCLUDE = 
WARN_ON = -Wcast-qual \
          -Wcast-align \
          -Wwrite-strings \
          -Wconversion \
          -Wfloat-equal \
          -Wpointer-arith \
          -Winit-self
#         -Weffc++
#         -Wunsafe-loop-optimizations \
#         -Wpadded \
CFLAGS = $(INCLUDE) -O2 -Wall -Wextra -Wformat=2 -Wstrict-aliasing=2 $(WARN_ON)
CXXFLAGS =$(INCLUDE) -O2 -Wall -Wextra -Wformat=2 -Wstrict-aliasing=2 $(WARN_ON)
LDFFAGS = 
LDLIBS = -lexpat

ifdef DEBUG
CFLAGS = $(INCLUDE) -g -Wall -Wextra -Wformat=2 -DDEBUG_
CXXFLAGS =$(INCLUDE) -g -Wall -Wextra -Wformat=2 -DDEBUG_
endif

SRCS = $(wildcard ./*.cpp)
OBJS = $(SRCS:.cpp=.o)

.SUFFIXES:
.SUFFIXES: .o .c .cpp

.PHONY: all clean depend

all: $(BIN)

$(BIN): $(OBJS)
	$(CXX) $(LDFFAGS) $(OBJS) -o $@ $(LDLIBS)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@

depend:
	$(CXX) -MM $(CXXFLAGS) $(SRCS) > dependencies
	cat dependencies

clean:
	-rm -f $(OBJS) $(BIN) *~

sinclude dependencies
