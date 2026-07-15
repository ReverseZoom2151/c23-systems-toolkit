.PHONY: configure build test check clean

configure:
	cmake -S . -B build

build: configure
	cmake --build build

test: build
	ctest --test-dir build --output-on-failure

check: test
	valgrind --leak-check=full --error-exitcode=1 ./build/binary-tests
	valgrind --leak-check=full --error-exitcode=1 ./build/list-tests
	valgrind --leak-check=full --error-exitcode=1 ./build/sketch-tests

clean:
	cmake -E rm -rf build
