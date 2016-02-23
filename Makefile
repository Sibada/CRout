####################################
#
# CRout的Makefile
# 2016-02-23
# Sibada
#
####################################


SHELL = /bin/bash

# 编译器
CXX = g++

# 普通的
CFLAGS = -I. -O3 -w
# 调试用
#CFLAGS = -I. -g -Wall


TARGET = crout

HEADS = crout.hpp \
	grid.hpp matrix.hpp \
	Time.hpp

SRCS = crout.cpp \
	initiate.cpp \
	unit_hydros.cpp \
	convolution.cpp \
	write_file.cpp


OBJS = $(SRCS:%.cpp=%.o) 




all : 
	make build
	make link

full :
	make clean
	make all

build : $(SRCS) $(HEADS)
	$(CXX) $(CFRAGS) -c $(SRCS)

link : $(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) -o $(TARGET)$(EXT)

.PHONY : clean
clean :
	\rm -f *.o $(TARGET)$(EXT)

