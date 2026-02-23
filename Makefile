default: compiler_make compile arm
.PHONY: default

compiler_cmake:
	mkdir -p ./out/CCompiler
	rm -rf ./out/CCompiler/*
	cmake -S . -B ./out/CCompiler
.PHONY: compiler_cmake

compiler_make:
	make -s -C ./out/CCompiler
.PHONY: compiler_make

compile:
	cd ./out/CCompiler && ./CCompiler ../../examples/test.c ./main.s
	arm-linux-gnueabi-as --warn --fatal-warnings -march=armv7-a -mcpu=cortex-a15 ./out/CCompiler/main.s -o ./out/CCompiler/main.o
	arm-linux-gnueabi-ld ./out/CCompiler/main.o -o ./out/CCompiler/main
.PHONY: compile

compile_gcc:
	arm-linux-gnueabi-gcc -S -O0 ./examples/test.c -o ./out/CCompiler/main.s
	arm-linux-gnueabi-gcc -c -march=armv7-a -mcpu=cortex-a15 ./out/CCompiler/main.s -o ./out/CCompiler/main.o
	arm-linux-gnueabi-gcc ./out/CCompiler/main.o -o ./out/CCompiler/main
.PHONY: compile_gcc

compile_ccomp:
	ccomp -S -O0 ./examples/test.c -o ./out/CCompiler/main.s
	arm-linux-gnueabi-gcc -c -march=armv7-a -mcpu=cortex-a15 ./out/CCompiler/main.s -o ./out/CCompiler/main.o
	arm-linux-gnueabi-gcc ./out/CCompiler/main.o -o ./out/CCompiler/main
.PHONY: compile_gcc

arm:
	cd ./out/CCompiler && qemu-arm -L /usr/arm-linux-gnueabi ./main
.PHONY: arm

test:
	python3 tests/testsuite.py -i tests/ -r
.PHONY: test

test_unit:
	python3 tests/testsuite.py -i tests/unit -r
.PHONY: test

