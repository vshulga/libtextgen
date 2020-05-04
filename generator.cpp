#include "generator.h"
#include <numeric>
#include <stdexcept>

using namespace text::generator;

namespace
{
    inline std::size_t random(std::size_t a, std::size_t b, std::default_random_engine& urng)
    {
        std::uniform_int_distribution<std::size_t> distribution(a, b);
        return distribution(urng);
    }

    struct exceptions
    {
        std::ios& ios;
        const std::ios_base::iostate except;
        exceptions(std::ios& ios, std::ios_base::iostate except) : ios(ios), except(ios.exceptions())
        { ios.exceptions(except); }
        ~exceptions()
        { ios.exceptions(except); }
    };

    struct header
    {
        std::size_t pref_size;
        std::size_t word_data_size;
        std::size_t word_index_size;
        std::size_t pref_data_size;
        std::size_t pref_index_size;
        std::size_t table_size;
        std::size_t checksum;

        std::size_t hash() const
        {
            const auto a = { pref_size, word_data_size, word_index_size,
                pref_data_size, pref_index_size, table_size };
            return std::accumulate(std::begin(a), std::end(a), std::size_t{},
                [h = std::hash<std::size_t>()] (auto l, auto r) { return l ^ h(r); });
        }
    };
}

std::size_t model::insert(const char* word)
{
    decltype(word_index)::iterator iter(word_index.lower_bound(word));
    if (iter == word_index.end() || word_index.key_comp()(word, *iter))
    {
        const auto pos(word_data.size());
        word_data.insert(word_data.end(), word, word + std::strlen(word) + 1);
        iter = word_index.emplace_hint(iter, pos);
    }
    return *iter;
}

std::size_t model::find(const char* word) const
{
    const auto iter(word_index.find(word));
    return iter == word_index.end() ? ~std::size_t() : *iter;
}

std::size_t model::insert(const std::list<std::size_t>& pref)
{
    decltype(pref_index)::iterator iter(pref_index.lower_bound(pref));
    if (iter == pref_index.end() || pref_index.key_comp()(pref, *iter))
    {
        auto first(pref.begin());
        auto pos(pref_data.size());
        if (0 < pref.size() && pref.size() <= pref_data.size() &&
            std::equal(pref.begin(), std::prev(pref.end()), pref_data.end() - pref.size() + 1))
        {
            first = std::prev(pref.end());
            pos = pref_data.size() - pref.size() + 1;
        }
        pref_data.insert(pref_data.end(), first, pref.end());
        iter = pref_index.emplace_hint(iter, pos);
    }
    return *iter;
}

std::size_t model::find(const std::list<std::size_t>& pref) const
{
    const auto iter(pref_index.find(pref));
    return iter == pref_index.end() ? ~std::size_t() : *iter;
}

void training::model::train(std::list<std::size_t>& state, const char* word)
{
    const auto word_pos(insert(word));
    const auto pref_pos(state.back());
    state.pop_back();
    if (!state.empty())
    {
        state.pop_front();
        state.push_back(word_pos);
    }
    state.push_back(insert(state));
    auto& word_stat(table[pref_pos][word_pos]);
    ++word_stat.first;
    word_stat.second = state.back();
}

void training::model::save(std::ostream& os) const
{
    const exceptions e(os, std::ios_base::failbit | std::ios_base::badbit);
    const std::ostream::sentry s(os);

    header h = { pref_size(), word_data.size(), word_index.size(),
        pref_data.size(), pref_index.size(), table.size()};
    h.checksum = h.hash();
    os.write(reinterpret_cast<const char*>(&h), sizeof(h));

    os.write(word_data.data(), word_data.size());
    std::for_each(word_index.begin(), word_index.end(), [&os] (auto v) {
        os.write(reinterpret_cast<const char*>(&v), sizeof(v));
    });
    os.write(reinterpret_cast<const char*>(pref_data.data()),
        sizeof(*pref_data.data())*pref_data.size());
    std::for_each(pref_index.begin(), pref_index.end(), [&os] (auto v) {
        os.write(reinterpret_cast<const char*>(&v), sizeof(v));
    });
    std::for_each(table.begin(), table.end(), [&os] (const auto& v) {
        const std::size_t a[] = { v.first, v.second.size() };
        os.write(reinterpret_cast<const char*>(a), sizeof(a));
        std::for_each(v.second.begin(), v.second.end(), [&os] (const auto& v) {
            const std::size_t a[] = { v.first, v.second.first, v.second.second };
            os.write(reinterpret_cast<const char*>(a), sizeof(a));
        });
    });
}

const char* generating::model::generate(std::list<std::size_t>& state,
    std::default_random_engine& urng) const
{
    const auto iter(table.find(state.back()));
    if (iter == table.end())
        return nullptr;
    const auto& second(iter->second);
    if (second.empty())
        throw std::invalid_argument("invalid prefix");
    const auto second_iter(second.lower_bound(random(1, second.rbegin()->first, urng)));
    if (second_iter == second.end())
        throw std::invalid_argument("invalid frequency");
    if (word_data.size() <= second_iter->second.first)
        throw std::invalid_argument("invalid word");
    state.back() = second_iter->second.second;
    return &word_data[second_iter->second.first];
}

void generating::model::load(std::istream& is)
{
    const exceptions e(is, std::ios_base::failbit | std::ios_base::badbit);
    const std::istream::sentry s(is, true);

    header h = {};
    is.read(reinterpret_cast<char*>(&h), sizeof(h));
    if (h.hash() != h.checksum)
        is.setstate(std::ios_base::failbit);

    word_data.resize(h.word_data_size);
    is.read(word_data.data(), word_data.size());
    std::generate_n(std::inserter(word_index, word_index.end()), h.word_index_size, [&is, &h] () {
        std::size_t v{};
        is.read(reinterpret_cast<char*>(&v), sizeof(v));
        if (h.word_data_size < v + 1)
            is.setstate(std::ios_base::failbit);
        return v;
    });
    pref_data.resize(h.pref_data_size);
    is.read(reinterpret_cast<char*>(pref_data.data()), sizeof(*pref_data.data())*pref_data.size());
    pref_index = decltype(pref_index)(lgcmp(pref_data, h.pref_size));
    std::generate_n(std::inserter(pref_index, pref_index.end()), h.pref_index_size, [&is, &h] () {
        std::size_t v{};
        is.read(reinterpret_cast<char*>(&v), sizeof(v));
        if (h.pref_data_size < v + h.pref_size)
            is.setstate(std::ios_base::failbit);
        return v;
    });
    std::generate_n(std::inserter(table, table.end()), h.table_size, [&is] () {
        std::size_t a[2] = {};
        is.read(reinterpret_cast<char*>(a), sizeof(a));
        decltype(table)::value_type::second_type second;
        std::generate_n(std::inserter(second, second.end()), a[1], [&is, sum = std::size_t{}] () mutable {
            std::size_t a[3] = {};
            is.read(reinterpret_cast<char*>(a), sizeof(a));
            return std::make_pair(sum += a[1], std::make_pair(a[0], a[2]));
        });
        return std::make_pair(a[0], std::move(second));
    });
}
