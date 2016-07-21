#/bin/sh

if test -d build ;
then rm -rf build
fi
mkdir build

cd build
cmake ..
cmake .. && make
cp flv_repeater /usr/local/bin/
