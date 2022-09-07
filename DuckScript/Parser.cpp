#include "Parser.h"
#include "Keypress.h"

template <typename T>
inline std::vector<T> split(T x, const char delim = ' ')
{
    x += delim; //includes a delimiter at the end so last word is also read
    std::vector<T> split = {};
    std::string temp = "";
    for(unsigned int i = 0; i < x.length(); i++)
    {
        if(x[i] == delim)
        {
            split.push_back(temp);
            temp = "";
            i++;
        }
        temp += x[i];
    }
    return split;
}

Parser::Parser()
{
}
/**
* DELAY 1000
* GUI r
* STRING notepad
* ENTER
*/

void Parser::parse(const std::vector<std::string>& lines)
{
    int curLine = 0;
    try {
        for(const auto& line : lines)
        {
            curLine++;
            std::cout << line << std::endl;
            std::vector<std::string> words = split(line);
            if(words.size() == 0 || words.at(0) == "\n" || words.at(0) == "\r")
                return;

            auto command = getCommand(words.at(0));
            if(command.has_value()) {
                switch(*command)
                {
                    case Parser::COMMANDS::DELAY:
                        if(words.size() != 2) {
                            throw std::runtime_error("ERROR: Expected one argument for DELAY");
                        }

                        int delay = std::stoi(words.at(1));
                        Sleep(delay);
                        std::cout << "Sleeping for " << delay << std::endl;
                        break;
                }
            } else {
                auto key = getKey(words.at(0));

                switch(*key)
                {
					case Parser::KEYS::GUI:
						if(words.size() != 2) {
							throw std::runtime_error("ERROR: Expected one argument for GUI");
						}

						keybd_event(VK_LWIN, 0, 0, 0);
						keybd_event(VK_LWIN, 0, KEYEVENTF_KEYUP, 0);
						break;
					case Parser::KEYS::ENTER:
						keybd_event(VK_RETURN, 0, 0, 0);
						keybd_event(VK_RETURN, 0, KEYEVENTF_KEYUP, 0);
						break;
                }
            }
        }
    } catch(std::exception ex) {
        ex = std::runtime_error(ex.what() + std::string(" on line: " + std::to_string(curLine)));
        throw ex;
    } catch(std::runtime_error ex) {
        ex = std::runtime_error(ex.what() + std::string(" on line: " + curLine));
        throw ex;
    }
}

std::optional<Parser::COMMANDS> Parser::getCommand(const std::string& str)
{
    auto it = commandsLookup.find(str);
    return it == commandsLookup.end() ? std::nullopt : std::make_optional(it->second);
}

std::optional<Parser::KEYS> Parser::getKey(const std::string& str)
{
    auto it = keysLookup.find(str);
    return it == keysLookup.end() ? std::nullopt : std::make_optional(it->second);
}

std::optional<Parser::OPERATORS> Parser::getOperator(const std::string& str)
{

}