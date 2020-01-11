#pragma once

#include "tools.h"

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
auto merge(Traits traits, TypeList<Ts...> ts, TypeList<Us...> us)
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

template<typename Traits, typename Tuple>
struct CallMerge
{
    static constexpr Traits traits;
    static constexpr Tuple tuple;

    constexpr CallMerge(Traits, Tuple){}

    auto operator()(int ignore) const
    {
        return ::CallMerge(
            traits,
            applySequence(
                std::make_index_sequence<std::tuple_size_v<Tuple> / 2>{},
                [=](auto... i)
                {
                    return std::make_tuple(
                        merge(
                            traits,
                            std::get<i * 2>(tuple),
                            std::get<i * 2 + 1>(tuple))...);
            }));
    }
};

template<typename Traits, typename Tuple>
constexpr auto mergeSortImpl(Traits traits, Tuple t)
{
    constexpr auto generations = intLog2(std::tuple_size_v<Tuple>);
    
    return applySequence(
        std::make_index_sequence<generations>{},
        [=](auto... i)
        {
            CallMerge cm{traits, t};
            return (
                cm >>= ... >>= [=](auto f){ return f(i); }
            ).tuple;
        });
}

template<typename Traits, typename... Ts>
constexpr auto mergeSort(Traits traits, TypeList<Ts...>)
{
    return std::get<0>(mergeSortImpl(traits, std::tuple<TypeList<Ts>...>{}));
}
