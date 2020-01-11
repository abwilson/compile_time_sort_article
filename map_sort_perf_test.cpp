#include <cstddef>
#include <utility>
#include <type_traits>
#include <functional>

#include "map_sort.cpp"

#include DATA_FILE

auto input = std::index_sequence<DATA>{};

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

auto y = mapSort<Traits>(seqToTypeList(input));


