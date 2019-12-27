#include <cstddef>
#include <utility>
#include <type_traits>
#include <functional>

#include DATA_FILE


template<typename... Members>
struct Map: Members... {};

template<std::size_t key, typename Value>
struct Pair { using type = Value; };

template<std::size_t key, typename Value>
decltype(auto) get(const Pair<key, Value>& result){ return result; }

template<typename M, std::size_t key>
using Get = typename std::decay_t<
    decltype(get<key>(std::declval<M>()))>::type;

template<typename... Ts> struct TypeList {};

template<typename Index, typename Traits, typename... Ts>
struct MapSortImpl;

template<std::size_t... index, typename Traits, typename... Ts>
struct MapSortImpl<std::index_sequence<index...>, Traits, Ts...>: Traits
{
    template<typename T>
    static constexpr auto value(){ return Traits::template value<T>(); }
    using Traits::compare;

    template<std::size_t pos, typename T>
    static constexpr auto rankOf()
    {
        auto countOfTsWithLesserValue = (compare(value<Ts>(), value<T>()) +...);
        auto countOfEqualTsPrecedingInList =
            ((value<T>() == value<Ts>() && index < pos) +...);
        return countOfTsWithLesserValue + countOfEqualTsPrecedingInList;
    }

    using Ranking = Map<Pair<rankOf<index, Ts>(), Ts>...>;

    static constexpr auto sort(){ return TypeList<Get<Ranking, index>...>{}; }
};

template<typename Traits, typename... Ts>
auto mapSort(TypeList<Ts...>)
{
    return MapSortImpl<std::index_sequence_for<Ts...>, Traits, Ts...>::sort();
}

template<int i>
using Int = std::integral_constant<int, i>;


struct Traits {
    template<typename T>
    static constexpr auto value(){ return T::value; }
    static constexpr std::less<> compare;
};

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

auto x = mapSort<Traits>(seqToTypeList(input));

int main()
{
}

