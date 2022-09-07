#include "Parser.h"
#include "Dictionary.h"
#include <fstream>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

int main()
{
    try {
        std::ifstream input;
        input.open("payload.txt");

        if(!input.is_open()) {
            std::cout << "Couldn't find" << fs::current_path() / "payload.txt" << std::endl;
        }

        Parser parser = Parser();
        std::vector<std::string> out;
        std::string line;
        while(std::getline(input, line))
        {
            out.emplace_back(line);
        }

        parser.parse(out);
    } catch(std::exception ex) {
		std::cout << ex.what() << std::endl;
    }
}