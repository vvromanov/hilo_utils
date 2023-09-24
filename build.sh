rm -fr build
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCOVERAGE=On
make -j8

