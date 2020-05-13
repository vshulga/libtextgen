#pragma once

#include <map>
#include <regex>
#include <string>
#include <tuple>
#include <vector>

namespace program
{
    // the class copies some ideas from python argparse module
    // most likely in a large project I choose boost::program_options
    // but here I decided to write my own implementation to avoid boost library dependency
    class arguments
    {
    public:
        explicit arguments(const std::regex& re = std::regex("-\\w")) : re(re) {}

        std::string get(const std::string& name) const;
        decltype(auto) get() const { return positional; }

        std::string help() const;

        void parse(int argc, char* argv[]);

        void add(const std::string& name, const std::string& help,
            const std::string& dflt = std::string(), const std::string& cnst = std::string())
        { formal[name] = std::make_tuple(help, dflt, cnst); }
        template<class T, class = std::enable_if_t<std::is_arithmetic<T>::value>>
        void add(const std::string& name, const std::string& help, const T& dflt, const T& cnst)
        { add(name, help, std::to_string(dflt), std::to_string(cnst)); }
        template<class T, class = std::enable_if_t<std::is_arithmetic<T>::value>>
        void add(const std::string& name, const std::string& help, const T& dflt)
        { add(name, help, std::to_string(dflt)); }

    private:
        const std::regex re;
        std::map<std::string, std::tuple<std::string, std::string, std::string>> formal;
        std::string program;
        std::map<std::string, std::string> optional;
        std::vector<std::string> positional;
    };
}
