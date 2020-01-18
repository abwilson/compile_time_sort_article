#include "flat_merge2.h"

using Input = TypeList<bool, char, short, int, double>;


struct EfficientLayoutTraits
{
    template<typename T>
    static constexpr auto value(Tag<T>){ return sizeof(T); }
    static constexpr std::less<> compare{};
} traits;

static_assert(
    std::is_same<
        decltype(merge(traits, TypeList{}, TypeList{})),
        TypeList<> >::value,
    "");

static_assert(
    std::is_same<
        decltype(merge(traits, TypeList{}, TypeList<int>{})),
        TypeList<int> >::value,
    "");

static_assert(
    std::is_same<
        decltype(merge(traits, TypeList<char>{}, TypeList<int>{})),
        TypeList<char, int> >::value,
    "");

static_assert(
    std::is_same<
       decltype(
           merge(
               traits,
               TypeList<bool, short, int>{},
               TypeList<char, short, int>{})),
       TypeList<char, bool, short, short, int, int> >::value,
    "");

// static_assert(
//     std::is_same_v<
//         decltype(mergeSort(traits, Input{})),
//         Input>,
//     "");


int main(){}
