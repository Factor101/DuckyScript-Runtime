#include "Parser.h"
#include <sstream>
#include <regex>

//TODO: evaluation of DEFINED constants in STRING commands

template <typename T>
inline std::vector<T> split(T x, const char delim = ' ', bool inclusive = false)
{
    //includes a delimiter at the end so last word is also read
    x += delim; 
    std::vector<T> split = {};
    std::string temp = "";
    for(unsigned int i = 0; i < x.length(); i++)
    {
        if(x[i] == delim)
        {
            split.emplace_back(inclusive ? temp + delim : temp);
			temp = "";
            i++;
        }
        temp += x[i];
    }
    return split;
}

// TODO: evaluate each expression
std::string evaluateGroupings(std::string str) // unfinished
{
    std::regex innermostGroup(R"(\(([^()]*)\))");
    std::cout << std::regex_search(str, innermostGroup);
    while(std::regex_search(str, innermostGroup))
    {
        std::cout << str << std::endl;
        std::smatch match;
        std::string out;

        while(std::regex_search(str, match, innermostGroup))
        {
            std::string expression = match.str(1);

            out += match.prefix();
            out += std::regex_replace(match[0].str(), innermostGroup, "_EVALUATED_");
            str = match.suffix();
        }
        str = out + str;
        out = "";
    }

    std::cout << str << std::endl;
    return str;
}

std::vector<std::string> splitChars(const std::string& x)
{
    std::vector<std::string> split = {};
    std::string temp = "";
    for(unsigned int i = 0; i <= x.length(); i++)
    {
        if(temp != "") {
            split.emplace_back(temp);
            temp = "";
        }
        temp += x[i];
    }
    return split;
}

Parser::Parser()
{
}

void Parser::parse(const std::vector<std::string>& lines)
{
    int curLine = 0;
    try {
        for(const auto& line : lines)
        {
            LINE:
            curLine++;
            std::cout << line << std::endl;
            std::vector<std::string> words = split(line);

            for(auto it = words.begin(); it != words.end(); ++it)
            {
                if(!std::regex_match(*it, std::regex(R"($\$[a-z]+[0-9a-z\-_]^)", std::regex_constants::ECMAScript | std::regex_constants::icase)))
                    continue;

                bool isFirst = it == words.begin();
                std::cout << *it << " matched!\n";

                if(isFirst && words.at(1) == "=") {
                    auto reservedVar = getReservedVar(*it);
                    if(reservedVar.has_value() && Parser::vars.contains(*it)) {
                        try {
                            //TODO: support $a = ($a + 1)
                            Parser::vars[*it] = std::stoi(words.at(2));
                        } catch(std::exception ex) {
                            throw std::runtime_error("Variables may only be assigned to integer values");
                        }
                    } else {
                        throw std::runtime_error("Identifier \"" + *it + "\" is undefined");
                    }

                    goto LINE;
                } else if(Parser::vars.contains(*it))  {
                    
                } else {

                }
            }
            std::vector<std::string> wordsSpaced = split(line, ' ', true);

            if(words.size() == 0 || words.at(0) == "\n" || words.at(0) == "\r")
                return;

            auto command = getCommand(words.at(0));
            std::vector<std::string> args = std::vector<std::string>(words.begin() + 1, words.end());
            std::vector<std::string> argsSpaced = std::vector<std::string>(wordsSpaced.begin() + 1, wordsSpaced.end());

            if(command.has_value()) {
                switch(*command)
                {
                    case Parser::COMMANDS::DELAY:
                    {
                        if(args.size() == 0)
                            throw std::runtime_error("ERROR: Expected one argument for DELAY");
                        

                        int delay = std::stoi(args.at(0));
                        Sleep(delay);
                        std::cout << "Sleeping for " << delay << std::endl;
                        break;
                    }

                    case Parser::COMMANDS::DEFINE:
                    {
                        if(args.size() != 2)
                            throw std::runtime_error("ERROR: Expected two arguments for DEFINE");

                        try {
                            Parser::constants[args.at(0)].reset(new Parser::Constant<int>(std::stoi(argsSpaced.at(1))));
                        } catch(std::exception ex) {
                            Parser::constants[args.at(0)].reset(new Parser::Constant<std::string>(argsSpaced.at(1)));
                        }

                        break;
                    }

                    case Parser::COMMANDS::VAR:
                    {
                        if(args.size() != 2)
                            throw std::runtime_error("Expected two arguments for DEFINE");

                        auto reservedVar = getReservedVar(args.at(1));
                        if(reservedVar.has_value())
                            throw std::runtime_error("Reserved variable \"" + args.at(1) + "\" may not be assigned using VAR");

                        Parser::vars[args.at(0)] = std::stoi(args.at(1));

                        break;
                    }

                    case Parser::COMMANDS::STRINGLN:
                    case Parser::COMMANDS::STRING:
                    {
                        if(args.size() < 1)
                            throw std::runtime_error("ERROR: Expected argument(s) for STRING");
                        
                        if(args.size() == 1 && Parser::constants.contains(args.at(0))) {
                            try {
                                int value = dynamic_cast<Parser::Constant<int>&>(*Parser::constants[args.at(0)]).value;
                                args.at(0) = std::to_string(value);
                            } catch(std::exception ex) {
                                std::string value = dynamic_cast<Parser::Constant<std::string>&>(*Parser::constants[args.at(0)]).value;
                                args.at(0) = value;
                            }
                            
                        }

                        std::stringstream ss;
                        for(std::string e : argsSpaced)
                            ss << e;

                        for(const auto& e : splitChars(ss.str()))
                        {
                            auto curKey = getKey(e);
                            if(!curKey.has_value())
                                throw std::runtime_error("Unexpected token " + e + " where a key was expected");

                            auto keypress = getKeypress(*curKey);
                            if(!keypress.has_value())
                                throw std::runtime_error("Unable to find a keystroke for " + e);

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

std::optional<Parser::RESERVED_VARS> Parser::getReservedVar(const std::string& str)
{
    auto it = reservedVarsLookup.find(str);
    return it == reservedVarsLookup.end() ? std::nullopt : std::make_optional(it->second);
}

std::optional<Parser::OPERATORS> Parser::getOperator(const std::string& str)
{

}