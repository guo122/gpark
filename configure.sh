#!/bin/bash

cd "$(dirname "$0")"

if [ ! -d "bin" ] ; then
   mkdir bin
fi

echo "CXXFLAGS=-std=c++1y -O3" > Makefile
echo "CXX=g++" >> Makefile
echo "" >> Makefile
echo "OUTBIN=bin/gpark" >> Makefile
echo "" >> Makefile
echo "OBJS=gpark/main.o gpark/3rd/sha1.o gpark/GDBMgr.o gpark/GFile.o gpark/GFileMgr.o gpark/GFileTree.o gpark/GPark.o gpark/GThreadHelper.o gpark/GTools.o" >> Makefile
echo "" >> Makefile
echo "all: \$(OBJS)" >> Makefile
echo "	\$(CXX) \$(CXXFLAGS) -o \$(OUTBIN) \$(OBJS)" >> Makefile
echo "" >> Makefile
echo "install:" >> Makefile
echo "	install bin/gpark  \$(HOME)/bin" >> Makefile
echo "" >> Makefile
echo "clean:" >> Makefile
echo "	rm -rf \$(OBJS) bin Solution Makefile .DS_Store" >> Makefile
echo "" >> Makefile
