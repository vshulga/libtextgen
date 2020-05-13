#pragma once

#include <locale>
#include <memory>
#include <regex>
#include <string>

namespace string
{
    inline decltype(auto) converter()
    {
        // I do not know another way to get the standard library implementation
        // of char <=> wchar_t conversion facet
        // (std::use_facet returns a reference but wstring_convert takes ownership of the facet)
        struct codecvt : std::codecvt_byname<wchar_t, char, std::mbstate_t>
        { codecvt() : codecvt_byname(std::locale().name()) {} };
        return std::make_shared<std::wstring_convert<codecvt>>();
    }

    // the function works with a character stream and do not require random access
    // so it can work with any character device (tapes for example)
    // we need wchar_t to work with encodings which require more than 1 byte per character
    // (UTF-8 for example, where Russian letters are encoded by 2 bytes)
    // I decided that wchar_t is the best possible cross-platform way to achieve this
    // we do not need wchar_t for encoding like Windows-1251 btw
    inline decltype(auto) search(std::wstreambuf* sb, const std::wregex& re)
    {
        // all members of the lambda are shared_ptr to make the lambda copyable
        // (regex_iterator is copyable but it does not copy the input string and the regex)
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
            // we have to read a line because regex_iterator does not work
            // with input_iterator (istreambuf_iterator)
            while (first == last && std::getline(*wis, *line))
                first = std::wsregex_iterator(line->begin(), line->end(), *wre);
            if (first == last)
                return std::string();
            // we implicitly use std::ctype for character classification here
            std::wstring result((first++)->str());
            // we explicitly use std::ctype for tolower here
            wct->tolower(&result[0], &result[result.size()]);
            return wsc->to_bytes(result);
        };
    }
}
