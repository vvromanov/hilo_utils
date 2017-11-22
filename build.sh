rm -fr build
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCOVERAGE=On
# -DTRAVIS=On
make VERBOSE=1 -j8

