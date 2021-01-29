#include <iostream>
#include <sstream>

#include "../src/l_terrain.hpp"


int main() {
	auto generator = vanquisher::SineTerrainGenerator(16.);

	auto terrain = vanquisher::Terrain(11, 32, generator, 2);

	double pos[2];
	double value;
	int i;
	std::string line, num;

	while (std::getline(std::cin, line)) {
		std::stringstream sline(line);
	
		for (i = 0; std::getline(sline, num, ' '); i++) {
			if (num.empty()) {
				i--;
				continue;
			}
		
			if (i < 2) {
				pos[i] = strtod(num.c_str(), NULL);
			}
		}

		if (i != 2) {
			std::cerr << "Aborting: expected 2 values, got " << i << std::endl;
			break;
		}

		value = terrain.get_height(pos[0], pos[1]);

		std::cout << pos[0] << "," << pos[1] << " -> " << value << std::endl;
	}
}
