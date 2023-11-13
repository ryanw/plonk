.PHONY: all format compile clean

all: compile

format:
	find app -type f -name "*.h" -o -name "*.cpp" | xargs clang-format -i
	find libs/plonk -type f -name "*.h" -o -name "*.cpp" | xargs clang-format -i

compile: compile-shaders
	mkdir -p build
	cd build ;\
	cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 .. && \
	make

compile-shaders:
	glslc -fshader-stage=frag shaders/simple.frag.glsl -o shaders/simple.frag.spv
	glslc -fshader-stage=vert shaders/simple.vert.glsl -o shaders/simple.vert.spv

clean:
	rm -rf build

run: compile
	./build/app/app

watch: compile
	find app libs/plonk shaders/*.glsl -type f | entr make compile

watch-tests: compile
	find app libs/plonk shaders/*.glsl -type f | entr make test

test: compile
	cd build ;\
	make test || \
	cat Testing/Temporary/LastTest.log
