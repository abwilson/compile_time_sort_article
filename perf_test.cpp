#include <utility>
#include <array>

#include DATA_FILE

template<typename Int, Int... values> constexpr auto
sort(std::integer_sequence<Int, values...>);

template<typename Values> struct SortImpl;

template<typename Int, Int... values>
constexpr auto sort(std::integer_sequence<Int, values...> sequence)
{
    return SortImpl<decltype(sequence)>::sort();
}

template<typename Int, Int... values>
struct SortImpl<std::integer_sequence<Int, values...> >
{
    static constexpr auto sort()
    {
        return sort(std::make_index_sequence<sizeof...(values)>{});
    }

    template<std::size_t... index>
    static constexpr auto sort(std::index_sequence<index...>)
    {
        return std::integer_sequence<Int, ith<index>()...>{};
    }
    
    template<std::size_t i>
    static constexpr auto ith()
    {
        Int result{};
        (
            (i >= rankOf<values>() && i < rankOf<values>() + count<values>() ? 
             result = values : Int{}),...);
        return result;
    }

    template<Int x>
    static constexpr auto rankOf() { return ((x > values) +...); }
    
    template<Int x>
    static constexpr auto count() { return ((x == values) +...); }
};

template<typename Int, Int... values>
constexpr auto operator==(
    std::integer_sequence<Int, values...>, 
    std::integer_sequence<Int, values...>) { return true; }

template<typename Int, Int... values, Int... others>
constexpr auto operator==(
    std::integer_sequence<Int, values...>, 
    std::integer_sequence<Int, others...>) { return false; }

template<typename Int, Int... values>
constexpr auto toArray(std::integer_sequence<Int, values...>)
{
    return std::array<Int, sizeof...(values)>{ values... };
}

auto input = std::index_sequence<
    DATA
    >{};

auto x = toArray(sort(input));

int main()
{
}
