endian: endian.cpp
	clang++ -O2 -std=gnu++11 -stdlib=libc++ -o endian endian.cpp