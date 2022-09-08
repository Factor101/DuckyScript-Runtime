#include "Parser.h"
#include <numeric>
#include <sstream>

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
            std::vector<std::string> args = std::vector<std::string>(words.begin() + 1, words.end());

            if(command.has_value()) {
                switch(*command)
                {
                    case Parser::COMMANDS::DELAY:
                    {
                        if(args.size() == 0) {
                            throw std::runtime_error("ERROR: Expected one argument for DELAY");
                        }

                        int delay = std::stoi(args.at(0));
                        Sleep(delay);
                        std::cout << "Sleeping for " << delay << std::endl;
                        break;
                    }
                    case Parser::COMMANDS::STRINGLN:
                    case Parser::COMMANDS::STRING:
                    {
                        if(args.size() < 1) {
                            throw std::runtime_error("ERROR: Expected argument(s) for STRING");
                        }

                        std::stringstream ss;
                        for(std::string e : args)
                        {
							ss << e << " ";
                        }
                        ss.seekp(-1, std::ios_base::end);
                        ss << '\0';

                        for(auto ch : ss.str())
                        {
                            if(ch == '\0')
                                break;
							
                            const std::string curChar = static_cast<std::string>(&ch);
                            
                            auto curKey = getKey(curChar);
                            if(!curKey.has_value())
                                throw std::runtime_error("Unexpected token " + curChar + " where a key was expected");

                            auto keypress = getKeypress(*curKey);
                            if(!keypress.has_value())
                                throw std::runtime_error("Unable to find a keystroke for " + ch);

                            keypress->keystroke();
                        }
                        if(*command == Parser::COMMANDS::STRINGLN)
                            getKeypress(Parser::KEYS::ENTER)->keystroke();
                    }
                }
            } else {
                auto key = getKey(words.at(0));
                if(key.has_value()) {
                    switch(*key)
                    {
                        case Parser::KEYS::GUI:
                        {
                            if(args.size() != 1)
                                throw std::runtime_error("ERROR: Expected one or more arguments for GUI");
                            
                            std::vector<Keypress<int>> keys;
                            for(auto& k : args)
                            {
                                auto curKey = getKey(k);
                                if(!curKey.has_value())
                                    throw std::runtime_error("Unexpected token " + k + " where a key was expected");

                                auto keypress = getKeypress(*curKey);
                                if(!keypress.has_value())
                                    throw std::runtime_error("Unable to find a keystroke for " + k);

                                keys.emplace_back(*keypress);
                            }
                            
                            auto gui = Parser::keypressLookup.at(Parser::KEYS::GUI);
                            gui.press();
                            for(Keypress<int>& k : keys)
                            {
                                k.press();
                            }
                            for(Keypress<int>& k : keys)
                            {
                                k.release();
                            }
                            gui.release();
                            
                            break;
                        }
                        default:
                        {
                            auto keypress = getKeypress(*key);
                            if(!keypress.has_value())
                                throw std::runtime_error("Unable to find a keystroke for " + words.at(0));

                            keypress->keystroke();
                        }
                    }
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

std::optional<Keypress<int>> Parser::getKeypress(KEYS k)
{
    auto it = keypressLookup.find(k);
    return it == keypressLookup.end() ? std::nullopt : std::make_optional(it->second);
}

std::optional<Parser::OPERATORS> Parser::getOperator(const std::string& str)
{

}