/**
In the previous article I described a way of doing compile time
sorting. One of the questions that came out of this was why would
anyone want to do that? The first answer is just for fun, it's just
pretty for it's own sake, and I think we as programmers ought to be
able to appreciate that even if there is no practicle use. To put it
another way, quoting Albert Eistein, what use is a new born baby?
Secondly, the sorting algorithm illustrates some techniques that can be
applied more generally to solve other problems - really it's just
playing with parameter pack expansion. Last but not least, there are
real reasons why you'd want to sort at compile time and in this
article I'm going to show you one.

Firstly though I'm going to show another little trick, apply it to
create yet another compile time sorting algorithm and then use that to
solve a practical problem.

A Simple Compile Time Map

Look at the following bit of code.
*/
template<typename... Members>
struct Map: Members... {};

/*
If we instantiate Map with a pack of types, (assuming we can derive
from all of the types) we end up with a type that is a composition of
these. Furthermore we also know we can implicitly cast from the
composed type to any of its bases. So given:
 */
struct A {}; struct B {}; struct C {};
auto abc = Map<A, B, C>{};
/*
Then I can select the A base of abc like this:
*/
inline decltype(auto) getA(const A& a){ return a; }

const A& a = getA(abc);
/*
And similarly for the other bases. Nothing particularly radical about
that. But what if our bases look like this?
*/
#include <cstddef>

template<std::size_t key, typename Value>
struct Pair { using type = Value; };
/*
Now we can write a generalised get.
*/
template<std::size_t key, typename Value>
inline constexpr decltype(auto) get(const Pair<key, Value>& result)
{ return result; }
/*
The crucial thing is that type deduction now comes into play. So given:
*/
using ABCMap = Map<Pair<0, A>, Pair<1, B>, Pair<2, C> >;
ABCMap abcMap;
/*
I can write:
*/
#include <utility>
#include <type_traits>

const auto& x = get<1>(abcMap);
/*
We have constrained key and, since our keys are unique, when the
compiler tries to deduce Value there is only one posible solution,
abcMap must be cast to const Pair<1, B>&. The expression is
unambiguous and x ends referencing the appropriate base of abcMap.
*/
static_assert(std::is_same<std::decay_t<decltype(x)>, Pair<1, B> >{},
              "get() pulls out the corresponding base.");
/*
We can translate this back to the world of types with this slightly
ugly incantation:
*/
template<typename M, std::size_t key>
using Get = typename std::decay_t<
    decltype(get<key>(std::declval<M>()))>::type;

static_assert(std::is_same<Get<ABCMap, 2>, C>{}, "Get works too.");
/*
So we have a very simple way of mapping from integers to types. The
key doesn't have to be a non-type and it doesn't have to be an int,
but it does have to be a compile time construct. There are other ways
of making compile time maps but I quite like this one as it's simple,
and uses the basic rules of C++ that we all (should) understand. This
map type is obvioulsy very similar to std::tuple - but we'll come back
to that later. The order we declare the pairs in the mapping is not
important. We could have said:
*/
using ABCMap2 = Map<Pair<2, C>, Pair<1, B>, Pair<0, A> >;
/*
Type deduction will still produce the same result - another MacGuffin.

Another Way to Sort

If we can rank each element of a set, then sorting is just forming a
mapping from rank to element. In other words a sorted sequence allows
us to access the elements by rank. Let's express that in code.

Simple type list to wrap a parameter pack.
*/
template<typename... Ts> struct TypeList {};

/*
Primary definition. We require an index_sequence, some traits to
generalize how we do the sort and some types to sort.
*/
template<typename Index, typename Traits, typename... Ts>
struct MapSortImpl;

/*
Partially specialize to break out the index_sequence. The index pack
gives us 0..sizeof..(Ts) - 1. This corresponds to the position of each
T in Ts. We'll need this throughout the following code.
*/
template<std::size_t... index, typename Traits, typename... Ts>
struct MapSortImpl<std::index_sequence<index...>, Traits, Ts...>: Traits
{
    /*
      Shorthand to use traits. Note we've derived from Traits to make
      this easier.
    */
    template<typename T>
    static constexpr auto value(){ return Traits::template value<T>(); }
    using Traits::compare;

    /*
      How do we find the rank of an element? Firstly we count up how
      many other elements rank lower than it using the traits compare
      function.

      We then consider equal ranking elements. We want a stable sort,
      so we add on the count of the equal elements preceding this
      element in the list. This keeps equal ranking elements in their
      existing order and ensures every element has a well defined
      place.
      
      Find the rank of element T which was at position pos in Ts.
     */
    template<typename T, std::size_t pos>
    static constexpr auto rankOf()
    {
        auto countOfTsWithLesserValue = (compare(value<Ts>(), value<T>()) +...);
        auto countOfEqualTsPrecedingInList =
            ((value<T>() == value<Ts>() && index < pos) +...);
        return countOfTsWithLesserValue + countOfEqualTsPrecedingInList;
    }
    /*
      Now we can define the ranking as a mapping from rank to T for
      each T in Ts.
    */
    using Ranking = Map<Pair<rankOf<Ts, index>(), Ts>...>;
    /*
      Can you dig it? What I'm saying here is Ranking is a map who's
      keys are the ranks (defined by rankOf()) of the corresponding
      Ts. In other words it's a map from sorted position to type.

      We can produce the sorted result by Getting from the ranking map
      in sequence.
    */
    static constexpr auto sort(){ return TypeList<Get<Ranking, index>...>{}; }
};

/*
Finally some syntactic sugar.
*/
template<typename Traits, typename... Ts>
inline constexpr auto mapSort(TypeList<Ts...>)
{
    return MapSortImpl<std::index_sequence_for<Ts...>, Traits, Ts...>::sort();
}
/*
Does it work?
*/
#include <functional>

template<int i>
using Int = std::integral_constant<int, i>;

struct Traits {
    template<typename T>
    static constexpr auto value(){ return T::value; }
    static constexpr std::less<> compare;
};

using In = TypeList<Int<42>, Int<7>, Int<13>, Int<7> >;
using Out = decltype(mapSort<Traits>(In{}));

static_assert(
    std::is_same<
        Out,
        TypeList<Int<7>, Int<7>, Int<13>, Int<42> > >{},
    "mapSort works!");

/*
What's the Point?

Consider:
*/
using T1 = std::tuple<bool, std::int16_t, char, std::int32_t, std::int64_t>;
static_assert(sizeof(T1) == 24, "pathalogical case std::tuple pads");

using T2 = std::tuple<std::int64_t, std::int32_t, std::int16_t, bool, char>;
static_assert(sizeof(T2) == 16, "efficient layout sorts by size");
/*
We all know that on most architectures types have an aligment. And we
ought to know that to get a space efficient layout we should sort our
members by size, biggest to smallest. Efficient layouts make better
use of cache and that makes code go faster.

In many cases where we define a std::tuple we can rearrange the
members by hand for efficiency, but sometime we might not be able to -
maybe the type is generated by TMP, or maybe we want future proof our
code against changes which effect the alignments. So can we come up
with something like a std::tuple but which automatically lays itself
out efficiently? Let's try.

Firstly we'll revisit the compile time map we started with, but with a
little tweak. Field now holds a value.
*/
template<std::size_t key, typename T>
struct Field { T value; };

/*
get() returns that value.
*/
template<std::size_t key, typename T>
decltype(auto) get(const Field<key, T>& f){ return (f.value); }

/*
We can actually reuse the Map template but the name no longer makes
sense so I'll rename it.
 */
template<typename... Ts>
using Tuple = Map<Ts...>;

/*
To prove this works I define the following.
*/
template<std::size_t key, typename T, typename Tuple>
constexpr auto checkType(Tuple&& t)
{
    return std::is_same<
        std::decay_t<decltype(get<key>(t))>,
        T>::value;
}

auto boolShort = Tuple<Field<0, bool>, Field<1, std::int16_t> >{};

static_assert(checkType<0, bool>(boolShort), "pulls out bool");
static_assert(checkType<1, std::int16_t>(boolShort), "pulls out short");
/*
So we've defined the very barest bones of a tuple template. I leave it
to the reader to flesh out the missing parts. The important difference
from std::tuple is that we explicitly associate a key with an
element. The keys could be given out of order and could be
non-contiguous, but the association between key and type is still
maintained. So we can define a tuple where the elements can be
reordered to form a space efficient layout? All we need to do now is
sort the types by size. Here's how we do that.

First we define an appropriate traits class.
*/
struct EfficientLayoutTraits
{
    /*
      We want to sort by size.
    */
    template<typename T>
    static constexpr auto value(){ return sizeof(T::value); }
    /*
      Biggest first.
    */
    static constexpr std::greater<> compare;
};

/*
A little helper that transforms a TypeList to a Tuple.
*/
template<typename... Ts>
auto typeListToTuple(TypeList<Ts...>){ return Tuple<Ts...>{}; }

/*
Shorthand to expand index_sequence. This little function deserves and
article in itself.
 */
template<std::size_t... index, typename F>
auto applySequence(std::index_sequence<index...>, F&& f)
{
    return f(std::integral_constant<std::size_t, index>{}...);
}

/*
Given some Ts create a Tuple with efficient layout.
*/
template<typename... Ts>
auto makeEfficientLayout()
{
    /*
      Form a list of Fields containing Ts, indexed by declaration order.
     */
    auto unsortedFields = applySequence(
        std::index_sequence_for<Ts...>{},        
        [](auto... index){ return TypeList<Field<index, Ts>...>{}; });
    /*
      Sort it using our traits.
     */
    auto sortedFields = mapSort<EfficientLayoutTraits>(unsortedFields);
    /*
      Turn it into a Tuple.
     */
    return typeListToTuple(sortedFields);
}

/*
Our previous pathalogical case.
*/
auto efficientTuple = makeEfficientLayout<
    bool, std::int16_t, char, std::int32_t, std::int64_t>();
/*
Tuple elements have been reordered to use minimum space.
 */
static_assert(sizeof(efficientTuple) == 16, "QED");

/*
But are accessed by original declaration order.
 */
static_assert(checkType<0, bool         >(efficientTuple));
static_assert(checkType<1, std::int16_t >(efficientTuple));
static_assert(checkType<2, char         >(efficientTuple));
static_assert(checkType<3, std::int32_t >(efficientTuple));
static_assert(checkType<4, std::int64_t >(efficientTuple));

int main(){}

/*
Footnote is it Any Good?

The sort I presented in the previous article - no. It's actaully
astonishingly bad. Never in all my time as a programmer have I burned
so many clock cycles on so simple a problem.

What about mapSort? That's actually much better but is still at least
n^2. Compared to a recursive nlog(n) algorithm mapSort is slower but
the recursive code hits the template recursion depth limit while
mapSort will happily keep on burning cycles.
 */
