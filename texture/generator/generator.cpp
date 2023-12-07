#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

inline static std::string bytes_to_array(unsigned char *data, size_t size) {
	std::stringstream ss;
	std::string ret;
	for (size_t i = 0; i < size; ++i) {
		ss << (int)data[i] << ',';
		if (ss.str().length() >= 80) {
			ret += ss.str() + '\n';
			ss.str("");
		}
	}
	ret += ss.str();
	return ret;
}

// usage: texture, output
int main(int argc, char **argv) {
	--argc, ++argv;
	if (argc != 2)
		return EXIT_FAILURE;

	std::basic_ifstream<char> fin{argv[0], std::ios::binary | std::ios::ate};
	if (!fin.is_open()) {
		printf("unable to open %s\n", argv[0]);
		return EXIT_FAILURE;
	}
	std::ifstream::pos_type size = fin.tellg();
	if (size == 0) {
		printf("empty texture %s\n", argv[0]);
		return EXIT_FAILURE;
	}
	std::vector<char> bytes(size);
	fin.seekg(0, std::ios::beg);
	fin.read(bytes.data(), size);

	std::ofstream fout{argv[1]};
	if (!fout.is_open()) {
		printf("unable to open %s\n", argv[1]);
		return EXIT_FAILURE;
	}
	fout << bytes_to_array((unsigned char *)bytes.data(), size);
}
