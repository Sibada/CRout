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

ARCFLAGS = -DARCINFO

DEFFLAGS = -DDEFAULT

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


arcinfo:
	make arcbuild
	make link
	

.PHONY : build
build : $(SRCS) $(HEADS)
	$(CXX) $(CFRAGS) $(DEFFLAGS) -c $(SRCS)

arcbuild : $(SRCS) $(HEADS)
	$(CXX) $(CFRAGS) $(ARCFLAGS) -c $(SRCS)

.PHONY : link
link : $(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) -o $(TARGET)$(EXT)

.PHONY : clean
clean :
	\rm -f *.o $(TARGET)$(EXT)

