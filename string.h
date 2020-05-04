#pragma once

#include <locale>
#include <memory>
#include <regex>
#include <string>

namespace string
{
    inline decltype(auto) converter()
    {
        struct codecvt : std::codecvt_byname<wchar_t, char, std::mbstate_t>
        { codecvt() : codecvt_byname(std::locale().name()) {} };
        return std::make_shared<std::wstring_convert<codecvt>>();
    }

    inline decltype(auto) search(std::wstreambuf* sb, const std::wregex& re)
    {
        const auto loc(std::make_shared<std::locale>());        
        const std::shared_ptr<const std::ctype<wchar_t>> wct(
            loc, &std::use_facet<std::ctype<wchar_t>>(*loc));
        const auto wsc(converter());
        const auto wis(std::make_shared<std::wistream>(sb));
        const auto wre(std::make_shared<std::wregex>(re));
        const auto line(std::make_shared<std::wstring>());
        std::wsregex_iterator first, last;
        return [wct, wsc, wis, wre, line, first, last] () mutable
        {
            while (first == last && std::getline(*wis, *line))
                first = std::wsregex_iterator(line->begin(), line->end(), *wre);
            if (first == last)
                return std::string();
            std::wstring result((first++)->str());
            wct->tolower(&result[0], &result[result.size()]);
            return wsc->to_bytes(result);
        };
    }
}
