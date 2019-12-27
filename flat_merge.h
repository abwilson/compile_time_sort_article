#pragma once

#include <utility>
#include <functional>
#include <type_traits>

template<std::size_t... index, typename F>
auto applySequence(std::index_sequence<index...>, F&& f)
{
    return f(std::integral_constant<std::size_t, index>{}...);
}

template<typename... Ts> struct TypeList {};

template<typename... Lhs, typename... Rhs>
auto operator+(TypeList<Lhs...>, TypeList<Rhs...>)
{
    return TypeList<Lhs..., Rhs...>{};
}

template<typename T>
struct Tag { using type = T; };

template<typename... Lhs, typename Rhs>
constexpr auto operator+(TypeList<Lhs...>, Tag<Rhs>)
{ return TypeList<Lhs..., Rhs>{}; }

template<typename... Ts>
constexpr auto operator==(TypeList<Ts...>, TypeList<Ts...>){ return true; }

template<typename... Ts, typename... Other>
constexpr auto operator==(TypeList<Ts...>, TypeList<Other...>){ return false; }

template<typename Head, typename... Tail>
constexpr auto head(TypeList<Head, Tail...>){ return Tag<Head>{}; }

template<typename Head, typename... Tail>
constexpr auto tail(TypeList<Head, Tail...>){ return TypeList<Tail...>{}; }

constexpr auto empty(TypeList<>){ return std::true_type{}; }

template<typename... Ts>
constexpr auto empty(TypeList<Ts...>){ return std::false_type{}; }

template<typename... Ts>
constexpr auto size(TypeList<Ts...>){ return sizeof...(Ts); }


template<int i>
using Int = std::integral_constant<int, i>;

template<typename A, typename F>
auto operator>>=(A a, F f) { return f(a); }

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

template<std::size_t generation, std::size_t position>
struct split
{
    template<typename... Ts>
    constexpr auto operator()(TypeList<Ts...> tl) const
    {
        using Tup = std::tuple<Ts...>;
        constexpr auto resultSize = 1 << (generation + 1);
        return applySequence(
            std::make_index_sequence<resultSize>{},
            [](auto... i)
            {
                return TypeList<Get<i + position * resultSize, Tup>...>{};
            });
    }
};

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
                MergeImpl{Traits{}, ts, us, TypeList<>{}} >>=
                ... >>=
                [=](auto f){ return f(i); }
            ).result;
        });
}

