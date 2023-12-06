#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

std::filesystem::path include_path;

inline std::string read_shader(const std::filesystem::path &file, uint32_t depth = 0u) {
	if (depth > 10) {
		printf("circular include in %s\n", file.c_str());
		exit(EXIT_FAILURE);
	}

	std::ifstream fin{file};
	if (!fin.is_open()) {
		printf("unable to open %s\n", file.c_str());
		exit(EXIT_FAILURE);
	}

	std::string shader, line;
	while (std::getline(fin, line)) {
		if (line.find("#include") != std::string::npos) {
			std::string include_file;
			bool start = false;
			for (char c : line) {
				if (c == '\"')
					start = !start;
				else if (start)
					include_file += c;
			}

			shader += "#define GLSL\n";
			shader += read_shader(include_path / include_file, depth + 1);
		} else {
			shader += line;
			shader += '\n';
		}
	}

	return shader;
}

// usage: shader, output, include directory
int main(int argc, char **argv) {
	--argc, ++argv;
	if (argc != 3)
		return EXIT_FAILURE;

	include_path = argv[2];

	std::string shader = read_shader(argv[0]);

	std::ofstream fout{argv[1]};
	if (!fout.is_open()) {
		printf("unable to open %s\n", argv[1]);
		return EXIT_FAILURE;
	}

	fout << "R\"(";
	fout << shader << ")\"";
}