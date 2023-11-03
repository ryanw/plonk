#include <iostream>
#include <plonk/plonk.h>

int main(int, char **) {
	std::cout << "1 + 2 = " << plonk::sum(1, 2) << '\n';
	return 0;
}
