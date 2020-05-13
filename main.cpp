#include "generator.h"
#include "io.h"
#include "iterator.h"
#include "program.h"
#include "string.h"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>

using namespace iterator;
using namespace program;
using namespace text::generator;

namespace
{
    inline std::shared_ptr<std::ios> set_rdbuf(std::shared_ptr<std::ios> file, std::ios& ios)
    {
        struct rdbuf
        {
            std::shared_ptr<std::ios> file;
            std::ios& ios;
            std::streambuf* backup;
            rdbuf(std::shared_ptr<std::ios> file, std::ios& ios)
                : file(file), ios(ios), backup(ios.rdbuf(file->rdbuf())) {}
            ~rdbuf() { ios.rdbuf(backup); }
        };
        const auto result(std::make_shared<rdbuf>(file, ios));
        return std::shared_ptr<std::ios>(result, result->file.get());
    }

    inline decltype(auto) download(const std::string& url)
    {
        return io::popen("curl -s " + url, "r");
    }

    void train(training::model& model, const std::vector<std::string>& urls,
        const std::wregex& re, std::size_t concurrency)
    {
        std::list<io::filebuf_ptr> files;
        for (auto iter = urls.begin(); iter != urls.end() || files.begin() != files.end();
            files.front().reset(), files.pop_front())
        {
            while (iter != urls.end() && files.size() < concurrency)
                files.push_back(download(*iter++));
            auto s(string::search(files.front().get(), re));
            std::for_each(ifunction_begin(s), ifunction_end(s),
                [t = train(model)] (const auto& s) mutable { t(s.c_str()); });
        }
    }

    void generate(const generating::model& model, const std::wstring& prefix,
        const std::wregex& re, std::size_t text_size)
    {
        std::wstringbuf psb(prefix);
        auto s(string::search(&psb, re));
        const std::vector<std::string> pref_list(ifunction_begin(s), ifunction_end(s));
        auto g(generate(model, pref_list));
        std::copy(ifunction_begin(g, std::size_t()), ifunction_end(g, text_size),
            std::ostream_iterator<const char*>(std::cout, " "));
    }
}

int main(int argc, char* argv[])
{
    try
    {
        // stdin, stdout, and stderr streams always open in text mode by default in msvc
        io::setmode(stdin, true);
        io::setmode(stdout, true);

        arguments args;
        args.add("-h", "print help", false, true);
        args.add("-t", "train model from text", false, true);
        args.add("-g", "generate text from model", false, true);
        args.add("-l", "global locale (user-preferred by default)");
        args.add("-i", "input file (stdin by default)");
        args.add("-o", "output file (stdout by default)");
        args.add("-r", "word regex", "\\w+");
        args.add("-n", "text prefix length", std::size_t(1));
        args.add("-c", "download concurrency", std::size_t(1000));
        args.add("-w", "generated text size", std::size_t(1000000));
        args.add("-p", "generated text prefix");
        args.parse(argc, argv);

        std::locale::global(std::locale(args.get("-l")));
        const auto converter(string::converter());
        const auto help_flag(std::stoi(args.get("-h")) != 0);
        const auto train_flag(std::stoi(args.get("-t")) != 0);
        const auto generate_flag(std::stoi(args.get("-g")) != 0);
        const auto urls(args.get());
        const auto iname(args.get("-i"));
        const auto oname(args.get("-o"));
        const std::wregex re(converter->from_bytes(args.get("-r")));
        const auto prefix(converter->from_bytes(args.get("-p")));
        const auto prefix_size(std::stoull(args.get("-n")));
        const auto concurrency(std::max(std::stoull(args.get("-c")), 1ull));
        const auto text_size(std::stoull(args.get("-w")));

        // replace std::cin/std::cout rdbufs if input/output files are provided        
        const auto ifile(iname.empty() ? std::shared_ptr<std::ios>() :
            set_rdbuf(std::make_shared<std::ifstream>(iname, std::ios_base::binary), std::cin));
        const auto ofile(oname.empty() ? std::shared_ptr<std::ios>() :
            set_rdbuf(std::make_shared<std::ofstream>(oname, std::ios_base::binary), std::cout));
        std::stringstream memfile;

        if (help_flag || !(train_flag || generate_flag))
            std::cerr << args.help() << std::endl;

        if (train_flag)
        {
            training::model model(prefix_size);
            train(model, urls, re, concurrency);
            model.save(generate_flag ? memfile : std::cout);
        }

        if (generate_flag)
        {
            generating::model model(prefix_size);
            model.load(train_flag ? memfile : std::cin);
            generate(model, prefix, re, text_size);
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
