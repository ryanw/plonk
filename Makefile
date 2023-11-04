.ONESHELL:
.PHONY: format

format:
	find app -type f -name "*.h" -o -name "*.cpp" | xargs clang-format -i
	find libs/plonk -type f -name "*.h" -o -name "*.cpp" | xargs clang-format -i
