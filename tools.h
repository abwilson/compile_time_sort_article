#pragma once

#include <utility>
#include <functional>
#include <type_traits>

template<std::size_t... index, typename F>
auto applySequence(std::index_sequence<index...>, F&& f)
{
    return f(std::integral_constant<std::size_t, index>{}...);
}

template<typename... Ts>
struct TypeList{};

template<typename... Lhs, typename... Rhs>
auto operator+(TypeList<Lhs...>, TypeList<Rhs...>)
{
    return TypeList<Lhs..., Rhs...>{};
}

template<typename T>
struct Tag { using type = T; };

template<typename T>
using Type = typename T::type;

template<typename T>
constexpr auto untag(Tag<T>){ return T{}; }

template<typename... Lhs, typename Rhs>
constexpr auto operator+(TypeList<Lhs...>, Tag<Rhs>)
{ return TypeList<Lhs..., Rhs>{}; }

template<typename... Ts>
constexpr auto operator==(TypeList<Ts...>, TypeList<Ts...>)
{ return std::true_type{}; }

template<typename... Ts, typename... Other>
constexpr auto operator==(TypeList<Ts...>, TypeList<Other...>)
{ return std::false_type{}; }

template<typename Head, typename... Tail>
constexpr auto head(TypeList<Head, Tail...>){ return Tag<Head>{}; }

template<typename Head, typename... Tail>
constexpr auto tail(TypeList<Head, Tail...>){ return TypeList<Tail...>{}; }

constexpr auto empty(TypeList<>){ return std::true_type{}; }

template<typename... Ts>
constexpr auto empty(TypeList<Ts...>){ return std::false_type{}; }

template<typename... Ts>
constexpr auto size(TypeList<Ts...>){ return sizeof...(Ts); }

template<typename... Ts, typename F>
constexpr auto applyTypes(TypeList<Ts...>, F&& f)
{
    return f(Tag<Ts>{}...);
}

template<typename... TLs>
auto flatten(TypeList<TLs...>)
{
    return (TLs{} + ...);
}

auto flatten(TypeList<> tl)
{
    return tl;
}

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

