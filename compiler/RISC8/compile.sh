export CXX="/usr/bin/g++-10"
rm -rf build
mkdir build
cmake -S . -B ./build
cd build
make
