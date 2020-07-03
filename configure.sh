#!/bin/bash

cd "$(dirname "$0")"

cmake -E make_directory "Solution/make" && cmake -E chdir "Solution/make" cmake -DCMAKE_BUILD_TYPE=Release ../../

echo "all:" > Makefile
echo "	cd Solution/make && make" >> Makefile
