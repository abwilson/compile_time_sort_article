#include "flat_merge.h"

using Input = TypeList<bool, char, short, int>;

static_assert(
    std::is_same<
        decltype(split<0, 0>{}(Input{})),
        TypeList<bool, char> >::value, "");

static_assert(
    std::is_same<
        decltype(split<0, 1>{}(Input{})),
        TypeList<short, int> >::value, "");

using Input2 = TypeList<
    bool, char, short, int, long long, long long, long long, int>;

using Input3 = TypeList<long long, long long, long long, int>;

static_assert(
    std::is_same<decltype(split<1, 0>{}(Input2{})), Input>::value, "");

static_assert(
    std::is_same<decltype(split<1, 1>{}(Input2{})), Input3>::value, "");


struct EfficientLayoutTraits
{
    template<typename T>
    static constexpr auto value(Tag<T>){ return sizeof(T); }
    static constexpr std::less<> compare{};
};

static_assert(
    std::is_same<
        decltype(merge<EfficientLayoutTraits>(TypeList<>{}, TypeList<>{})),
        TypeList<> >::value,
    "");

static_assert(
    std::is_same<
        decltype(merge<EfficientLayoutTraits>(TypeList<>{}, TypeList<int>{})),
        TypeList<int> >::value,
    "");

static_assert(
    std::is_same<
        decltype(
            merge<EfficientLayoutTraits>(TypeList<char>{}, TypeList<int>{})),
        TypeList<char, int> >::value,
    "");

static_assert(
    std::is_same<
       decltype(
            merge<EfficientLayoutTraits>(
                TypeList<bool, short, int>{},
                TypeList<char, short, int>{})),
       TypeList<char, bool, short, short, int, int> >::value,
    "");

