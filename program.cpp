#include "program.h"
#include <algorithm>
#include <iterator>
#include <sstream>
#include <stdexcept>

using namespace program;

std::string arguments::get(const std::string& name) const
{
    const auto i(optional.find(name));
    if (i != optional.end())
        return i->second;
    const auto j(formal.find(name));
    if (j != formal.end())
        return std::get<1>(j->second);
    throw std::invalid_argument(help());
}

std::string arguments::help() const
{
    std::ostringstream r;
    r << "Usage: " << program << " [options] ..." << std::endl;
    r << "Options:" << std::endl;
    std::for_each(formal.begin(), formal.end(), [&r] (const auto& o) {
        r << '\t' << o.first << '\t' << std::get<0>(o.second);
        const auto& dflt(std::get<1>(o.second));
        if (!dflt.empty())
            r << " (" << dflt << " by default)";
        r << std::endl;
    });
    return r.str();
}

void arguments::parse(int argc, char* argv[])
{
    decltype(optional) opt;
    decltype(positional) pos;

    program = 0 < argc ? argv[0] : "";

    for (int i = 1; i < argc; ++i)
    {
        std::cmatch m;
        if (std::regex_match(argv[i], m, re))
        {
            const auto j(formal.find(argv[i]));
            if (j == formal.end())
                throw std::invalid_argument(help());
            const auto& cnst(std::get<2>(j->second));
            if (cnst.empty())
            {
                if (++i == argc)
                    throw std::invalid_argument(help());
                opt[j->first] = argv[i];
            }
            else
            {
                opt[j->first] = cnst;
            }
        }
        else
        {
            std::copy(argv + i, argv + argc, std::back_inserter(pos));
            break;
        }
    }

    std::swap(opt, optional);
    std::swap(pos, positional);
}
