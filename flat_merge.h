#pragma once

#include "tools.h"

template<typename Int>
constexpr auto intLog2(Int x)
{
    for(int i = 0;; x >>= 1, i++)
    {
        if(!x)
        {
            return i - 1;
        }
    }
}

template<std::size_t idx, typename T>
using Get = std::tuple_element_t<idx, T>;

template<std::size_t generation, std::size_t position, typename... Ts>
constexpr auto split(TypeList<Ts...> tl)
{
    using Tup = std::tuple<Ts...>;
    constexpr auto resultSize = 1 << generation;
    return applySequence(
        std::make_index_sequence<resultSize>{},
        [](auto... i)
        {
            return TypeList<Get<i + position * resultSize, Tup>...>{};
        });
}

template<typename Traits, typename L, typename R, typename Result>
struct MergeImpl
{
    static constexpr Traits traits;
    static constexpr L left;
    static constexpr R right;
    static constexpr Result result;

    MergeImpl(Traits, L, R, Result){}

    constexpr auto advanceLeft() const
    {
        return ::MergeImpl{traits, tail(left), right, result + head(left)};
    }

    constexpr auto advanceRight() const
    {
        return ::MergeImpl{traits, left, tail(right), result + head(right)};
    }

    constexpr auto operator()(int i) const
    {
        if constexpr(empty(left))
        {
            return advanceRight();
        }
        else if constexpr(empty(right))
        {
            return advanceLeft();
        }
        else if constexpr(
            traits.compare(
                traits.value(head(left)),
                traits.value(head(right))))
        {
            return advanceLeft();
        }
        else
        {
            return advanceRight();
        }
    }
};

template<typename Traits, typename... Ts, typename... Us>
constexpr auto merge(TypeList<Ts...> ts, TypeList<Us...> us)
{
    return applySequence(
        std::make_index_sequence<size(ts) + size(us)>{},
        [=](auto... i)
        {
            return (
                MergeImpl{Traits{}, ts, us, TypeList{}} >>=
                ... >>= [=](auto f){ return f(i); }
            ).result;
        });
}

template<std::size_t generation, typename Traits, typename... Ts>
auto splitMerge(TypeList<Ts...> ts)
{
    constexpr auto resultSize = 1 << (generation + 1);
    const auto log2Size = intLog2(sizeof...(Ts));
    const auto splits = 1 << (log2Size - generation - 1);
    
    return applySequence(
        std::make_index_sequence<splits>{},
        [=](auto... i)
        {
            return (
                merge(
                    split<generation, i>(ts),
                    split<generation, i + 1>(ts)) +...);
        });
}

