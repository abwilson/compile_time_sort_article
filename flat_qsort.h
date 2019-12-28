#pragma once

#include "tools.h"

template<typename Traits, typename T, typename U>
constexpr auto compare(Traits traits, Tag<T> t, Tag<U> u)
{
    return traits.compare(traits.value(t), traits.value(u));
}

template<typename Traits>
constexpr auto grade(Traits, TypeList<> t)
{
    return t;
}

template<typename Traits, typename T>
constexpr auto grade(Traits, TypeList<T> t)
{
    return TypeList<TypeList<T> >{};
}

template<typename Traits, typename T, typename U>
constexpr auto grade(Traits traits, TypeList<T, U> t)
{
    return std::conditional_t<
        compare(traits, Tag<T>{}, Tag<U>{}),
        TypeList<TypeList<T>, TypeList<U> >,
        TypeList<TypeList<U>, TypeList<T> > >{};
}

template<typename Traits, typename Pivot, typename Higher, typename Lower>
struct GradeImpl
{
    static constexpr Traits traits;
    static constexpr Pivot  pivot;
    static constexpr Higher higher;
    static constexpr Lower  lower;

    constexpr GradeImpl(Traits, Pivot, Higher, Lower){}
    
    template<typename T>
    constexpr auto operator()(Tag<T> t) const
    {
        if constexpr(compare(traits, pivot, t))
        {
            return ::GradeImpl(traits, pivot, higher + t, lower);
        }
        else
        {
            return ::GradeImpl(traits, pivot, higher, lower + t);
        }
    }
};

template<typename Traits, typename... Ts>
constexpr auto grade(Traits traits, TypeList<Ts...> ts)
{
    auto pivot = head(ts);
    auto toGrade = tail(ts);

    auto graded = applyTypes(
        toGrade,
        [=](auto... ts)
        {
            return (
                GradeImpl{traits, pivot, TypeList{}, TypeList{}} >>=
                ... >>= [=](auto f){ return f(ts); });
        });

    return TypeList<
        std::decay_t<decltype(graded.lower)>,
        TypeList<Type<decltype(pivot)> >,
        std::decay_t<decltype(graded.higher)> >{};
}

template<typename Traits, typename TLs>
struct FlatQsortImpl
{
    static constexpr Traits traits;
    static constexpr TLs tls;
    
    FlatQsortImpl(Traits, TLs){}
    
    constexpr auto operator()(int ignore)
    {
        return ::FlatQsortImpl(
            traits,
            applyTypes(
                tls,
                [](auto... tl){ return (grade(traits, untag(tl)) + ...); }));
    }
};

template<typename Traits, typename TL>
constexpr auto flatQsort(Traits traits, TL tl)
{
    auto result = applySequence(
        std::make_index_sequence<size(tl)>{},
        [=](auto... i)
        {
            return (
                FlatQsortImpl{traits, TypeList<TL>{}} >>=
                ... >>= [=](auto f){ return f(i); }
            );
        });
    
    return flatten(result.tls);
}
