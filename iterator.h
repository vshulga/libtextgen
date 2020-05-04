#pragma once

#include <iterator>
#include <utility>

namespace iterator
{
    template<class F, class S>
    class ifunction : public std::iterator<std::input_iterator_tag, decltype(std::declval<F>()())>
    {
    public:
        using function_type = F;
        using state_type = S;
        using value_type = typename ifunction<F, S>::value_type;

        ifunction(function_type& f, const state_type& s, const value_type& v)
            : function(&f)
            , state(s)
            , value(v)
        {
        }

        const value_type& operator*() const
        {
            return value;
        }

        const value_type* operator->() const
        {
            return &**this;
        }

        ifunction& operator++()
        {
            value = (*function)();
            ++state;
            return *this;
        }

        ifunction operator++(int)
        {
            ifunction result(*this);
            ++*this;
            return result;
        }

        bool operator==(const ifunction& right) const
        {
            return function == right.function && (state == right.state || value == right.value);
        }

        bool operator!=(const ifunction& right) const
        {
            return !(*this == right);
        }

    private:
        function_type* function;
        state_type state;
        value_type value;
    };

    struct infinite
    {
        infinite& operator++()
        {
            return *this;
        }

        infinite& operator++(int)
        {
            return *this;
        }

        bool operator==(const infinite&) const
        {
            return false;
        }
    };

    template<class F, class S = infinite>
    inline auto ifunction_begin(F& f, const S& s = S())
    {
        return ifunction<F, S>(f, s, f());
    }

    template<class F, class S = infinite, class V = typename ifunction<F, S>::value_type>
    inline auto ifunction_end(F& f, const S& s = S(), const V& v = V())
    {
        return ifunction<F, S>(f, s, v);
    }
}
