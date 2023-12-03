#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

// usage: shader, output, include directory
int main(int argc, char **argv) {
	--argc, ++argv;
	if (argc != 3)
		return EXIT_FAILURE;

	std::string line;

	std::string shader;

	std::filesystem::path include_path = argv[2];

	std::ifstream fin{argv[0]};
	if (!fin.is_open()) {
		printf("unable to open %s\n", argv[0]);
		return EXIT_FAILURE;
	}

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

			std::ifstream include_fin{include_path / include_file};
			if (!include_fin.is_open()) {
				printf("unable to include \"%s\"\n", include_file.c_str());
				return EXIT_FAILURE;
			}

			shader += {std::istreambuf_iterator<char>(include_fin), std::istreambuf_iterator<char>{}};
		} else {
			shader += line;
			shader += '\n';
		}
	}

	std::ofstream fout{argv[1]};
	if (!fout.is_open()) {
		printf("unable to open %s\n", argv[1]);
		return EXIT_FAILURE;
	}

	fout << "R\"(";
	fout << shader << ")\"";
}