.ONESHELL:
.PHONY: all format compile clean

all: compile

format:
	find app -type f -name "*.h" -o -name "*.cpp" | xargs clang-format -i
	find libs/plonk -type f -name "*.h" -o -name "*.cpp" | xargs clang-format -i

compile:
	mkdir -p build
	cd build
	cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 ..
	make

clean:
	rm -rf build

run: compile
	./build/app/app

watch: compile
	find app libs/plonk -type f | entr make compile
