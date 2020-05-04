#include "io.h"
#include <cerrno>
#include <system_error>

#ifdef __GNUC__
#include <ext/stdio_filebuf.h>
struct filebuf : __gnu_cxx::stdio_filebuf<wchar_t>
{ filebuf(std::FILE* f) : stdio_filebuf(f, std::ios_base::in | std::ios_base::out) {} };
#else
#include <fstream>
using filebuf = std::wfilebuf;
#endif

#ifdef _MSC_VER
#include <fcntl.h>
#include <io.h>
const auto& popen(_popen);
const auto& pclose(_pclose);
#endif

namespace
{
    inline std::system_error last_error(const std::string& context)
    {
        return std::system_error(errno, std::generic_category(), context);
    }
}

io::filebuf_ptr::filebuf_ptr(std::FILE* f, close_type* c)
    : close_result(std::make_unique<int>(0))
    , file(f, [&r = *close_result, c] (auto f) { return r = c(f); })
    , buffer(std::make_unique<filebuf>(f))
{
    buffer->pubimbue(std::locale());
}

std::wstreambuf* io::filebuf_ptr::get() const noexcept
{
    return buffer.get();
}

void io::filebuf_ptr::reset()
{
    buffer.reset();
    file.reset();
    if (const int r = *close_result)
        throw std::runtime_error("file close error " + std::to_string(r));
}

io::filebuf_ptr io::popen(const std::string& c, const std::string& m)
{
    const auto file(::popen(c.c_str(), m.c_str()));
    if (file == nullptr)
        throw last_error(__func__);
    return filebuf_ptr(file, &::pclose);
}

bool io::setmode(std::FILE* f, bool binary)
{
#ifdef _MSC_VER
    const int r(_setmode(_fileno(f), binary ? _O_BINARY : _O_TEXT));
    if (r == -1)
        throw last_error(__func__);
    return (r & _O_BINARY) == _O_BINARY;
#endif
    return binary;
}
