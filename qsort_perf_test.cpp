#include "flat_qsort.h"

#include DATA_FILE

template<int i>
using Int = std::integral_constant<int, i>;

auto input = std::index_sequence<
    DATA
    >{};

template<std::size_t... is>
auto seqToTypeList(std::index_sequence<is...>)
{
    return TypeList<Int<is>...>{};
}

template<typename Int, Int... values>
constexpr auto toArray(std::integer_sequence<Int, values...>)
{
    return std::array<Int, sizeof...(values)>{ values... };
}

struct SimpleTraits
{
    template<typename T>
    static constexpr auto value(Tag<T>){ return T::value; }
    static constexpr std::less<> compare{};
} traits;


auto x = flatQsort(traits, seqToTypeList(input));

int main()
{
}
