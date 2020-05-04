#pragma once

#include <cstdio>
#include <functional>
#include <memory>
#include <streambuf>
#include <string>

namespace io
{
    class filebuf_ptr
    {
        using close_type = int (std::FILE*);
    public:
        filebuf_ptr(std::FILE* f, close_type* c);
        std::wstreambuf* get() const noexcept;
        void reset();

    private:
        std::unique_ptr<int> close_result;
        std::unique_ptr<std::FILE, std::function<close_type>> file;
        std::unique_ptr<std::wstreambuf> buffer;
    };

    filebuf_ptr popen(const std::string& c, const std::string& m);

    bool setmode(std::FILE* f, bool binary);
}
