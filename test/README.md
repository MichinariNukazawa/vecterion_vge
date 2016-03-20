
## download
	git clone --depth=1 https://github.com/google/googletest.git

## build googletest
	cd googletest/googletst/
	cmake
	make

## move googletest libs
	libgtest.a libgtest_main.a to test/

## build and run test
	make -f test/Makefile run

