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
auto operator+(TypeList<Lhs...>, Tag<Rhs>){ return TypeList<Lhs..., Rhs>{}; }

template<typename... Ts>
constexpr auto operator==(TypeList<Ts...>, TypeList<Ts...>){ return true; }

template<typename... Ts, typename... Other>
constexpr auto operator==(TypeList<Ts...>, TypeList<Other...>){ return false; }

template<int i>
using Int = std::integral_constant<int, i>;

template<typename F, typename G>
auto operator>>=(F f, G g) { return [f, g](auto arg){ return g(f(arg)); }; }

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
                return TypeList<std::tuple_element_t<i + position * resultSize, Tup>...>{};
            });
    }
};

using Input = TypeList<bool, char, short, int>;

static_assert(std::is_same<decltype(split<0, 0>{}(Input{})), TypeList<bool, char> >::value, "");
static_assert(std::is_same<decltype(split<0, 1>{}(Input{})), TypeList<short, int> >::value, "");

using Input2 = TypeList<bool, char, short, int, long long, long long, long long, int>;
using Input3 = TypeList<long long, long long, long long, int>;
static_assert(std::is_same<decltype(split<1, 0>{}(Input2{})), Input>::value, "");
static_assert(std::is_same<decltype(split<1, 1>{}(Input2{})), Input3>::value, "");

template<std::size_t idx, typename T>
using Get = std::tuple_element_t<idx, T>;

template<std::size_t idx, typename Tuple, typename = void>
struct TupleIter: std::false_type {};

template<std::size_t idx, typename Tuple>
struct TupleIter<idx, Tuple, std::enable_if_t<(idx < std::tuple_size_v<Tuple>) > >:
    std::true_type
{
    using type = std::tuple_element_t<idx, Tuple>;
    static constexpr auto advance(){ return TupleIter<idx + 1, Tuple>{}; }
    static constexpr auto tag(){ return Tag<type>{}; }
};

template<typename L, typename R, typename Result>
struct Monad
{
    L left;
    R right;
    Result result;

    Monad(L l, R r, Result res): left(l), right(r), result(res){}

    constexpr auto advanceLeft()
    {
        return ::Monad{left.advance(), right, result + left.tag()};
    }

    constexpr auto advanceRight()
    {
        return ::Monad{left, right.advance(), result + right.tag()};
    }
};

template<typename Traits, typename Ts, typename Us>
struct Merge
{
    template<std::size_t>
    struct MergeImpl
    {
        template<typename State>
        constexpr auto operator()(State s) const
        {
            if constexpr(s.left)
            {
                if constexpr(s.right)
                {                
                    if constexpr(Traits::compare(Traits::value(s.left.tag()), Traits::value(s.right.tag())))
                    {
                        return s.advanceLeft();
                    }
                    else
                    {
                        return s.advanceRight();
                    }
                }
                else
                {
                    return s.advanceLeft();
                }
            }
            else
            {
                return s.advanceRight();
            }
        }
    };

    constexpr auto operator()()
    {
        return applySequence(
            std::make_index_sequence<std::tuple_size_v<Ts> + std::tuple_size_v<Us> >{},
            [](auto... i)
            {
                return (MergeImpl<i>{} >>= ... >>= [](auto x){ return x; })(
                    Monad{TupleIter<0, Ts>{}, TupleIter<0, Us>{}, TypeList<>{}}).result;
            });
    }
};

template<typename Traits, typename... Ts, typename... Us>
constexpr auto merge(TypeList<Ts...>, TypeList<Us...>)
{
    return Merge<Traits, std::tuple<Ts...>, std::tuple<Us...> >{}();
}

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


int main(){}
