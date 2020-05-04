#pragma once

#include <algorithm>
#include <cstring>
#include <istream>
#include <iterator>
#include <list>
#include <map>
#include <random>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

namespace text
{
    namespace generator
    {
        class model
        {
        public:
            explicit model(std::size_t pref_size)
                : word_index(strcmp(word_data))
                , pref_index(lgcmp(pref_data, pref_size)) {}

            std::size_t pref_size() const { return pref_index.key_comp().size; }

            std::size_t insert(const char* word);
            std::size_t find(const char* word) const;

            std::size_t insert(const std::list<std::size_t>& pref);
            std::size_t find(const std::list<std::size_t>& pref) const;

        protected:
            struct strcmp
            {
                using is_transparent = int;

                const std::vector<char>& data;

                explicit strcmp(const std::vector<char>& data) : data(data) {}

                bool operator()(std::size_t l, std::size_t r) const { return std::strcmp(data.data() + l, data.data() + r) < 0; }
                bool operator()(std::size_t l, const char* r) const { return std::strcmp(data.data() + l, r) < 0; }
                bool operator()(const char* l, std::size_t r) const { return std::strcmp(l, data.data() + r) < 0; }
            };

            struct lgcmp
            {
                using is_transparent = int;

                const std::vector<std::size_t>* data;
                std::size_t size;

                lgcmp(const std::vector<std::size_t>& data, std::size_t size) : data(&data), size(size) {}

                template<class L, class R>
                bool compare(L lf, L ll, R rf, R rl) const { return std::lexicographical_compare(lf, ll, rf, rl); }
                bool operator()(std::size_t l, std::size_t r) const
                { return compare(std::begin(*data) + l, std::begin(*data) + l + size, std::begin(*data) + r, std::begin(*data) + r + size); }
                template<class R>
                bool operator()(std::size_t l, const R& r) const
                { return compare(std::begin(*data) + l, std::begin(*data) + l + size, std::begin(r), std::end(r)); }
                template<class L>
                bool operator()(const L& l, std::size_t r) const
                { return compare(std::begin(l), std::end(l), std::begin(*data) + r, std::begin(*data) + r + size); }
            };

        protected:
            std::vector<char> word_data;
            std::set<std::size_t, strcmp> word_index;
            std::vector<std::size_t> pref_data;
            std::set<std::size_t, lgcmp> pref_index;
            std::unordered_map<std::size_t, std::map<std::size_t, std::pair<std::size_t, std::size_t>>> table;
        };

        namespace training
        {
            class model : public generator::model
            {
            public:
                using generator::model::model;

                void train(std::list<std::size_t>& state, const char* word);

                void save(std::ostream& os) const;
            };
        }

        namespace generating
        {
            class model : public generator::model
            {
            public:
                using generator::model::model;

                const char* generate(std::list<std::size_t>& state,
                    std::default_random_engine& urng) const;

                void load(std::istream& is);
            };
        }

        inline decltype(auto) first_prefix(std::size_t pref_size)
        {
            return std::vector<std::string>(pref_size);
        }

        inline decltype(auto) state(training::model& m, const std::vector<std::string>& pref_list)
        {
            std::list<std::size_t> result;
            std::transform(pref_list.begin(), pref_list.end(), std::back_inserter(result),
                [&m] (const auto& w) { return m.insert(w.c_str()); });
            result.push_back(m.insert(result));
            return result;
        }

        inline decltype(auto) state(const generating::model& m, const std::vector<std::string>& pref_list)
        {
            std::list<std::size_t> result;
            std::transform(pref_list.begin(), pref_list.end(), std::back_inserter(result),
                [&m] (const auto& w) { return m.find(w.c_str()); });
            result.push_back(m.find(result));
            return result;
        }

        inline decltype(auto) train(training::model& m)
        {
            return [&m, state = state(m, first_prefix(m.pref_size()))] (const char* word) mutable
            {
                m.train(state, word);
            };
        }

        inline decltype(auto) generate(const generating::model& m, const std::vector<std::string>& pref_list)
        {
            return [&m, state = state(m, pref_list.empty() ? first_prefix(m.pref_size()) : pref_list),
                urng = std::default_random_engine()] () mutable
            {
                return m.generate(state, urng);
            };
        }
    }
}
