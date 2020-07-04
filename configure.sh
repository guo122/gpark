#!/bin/bash

cd "$(dirname "$0")"

cmake -E make_directory "Solution/make" && cmake -E chdir "Solution/make" cmake -DCMAKE_BUILD_TYPE=Release ../../

link_info=`cat Solution/make/*/CMakeFiles/*.dir/link.txt`
bin_path="bin/"${link_info##*-o\ ..\/..\/..\/bin\/}
bin_path=${bin_path%%-L*}

echo "all:" > Makefile
echo "	cd Solution/make && make" >> Makefile
echo "" >> Makefile
echo "install:" >> Makefile
echo "	install $bin_path \$(HOME)/bin" >> Makefile
