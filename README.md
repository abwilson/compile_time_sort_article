# Non-Recursive Compile Time Sort

There was a time when compile time sorting was a holy grail for
me. It's still quite a tricky piece of meta programming to pull
off. When I came up with this idea I had a quick look around for prior
art including Stack Overflow where I first posted this
code. (https://stackoverflow.com/questions/32660523/c-calculate-and-sort-vector-at-compile-time). The
usual approach involves a set of recursively defined meta
functions. Since C++ 11 introduced parameter packs and C++ 14 added
std::index_sequence to the library things have go a bit simpler. This
is non-recursive compile time sort algorithm which I think is much
simpler and easier to understand. I like to think of parameter pack
expansion as a way of saying "for each do this". This algorithm
depends on thinking in these terms - thinking about how you can apply
a series of simple operations to all elements of a pack at once and
end up with a sorted sequence.

## The Problem

I'm going to define a constexpr function taking a
std::integer_sequence and returning a sorted version of the
sequence. I've deliberately removed as many complications as possible
in order to make the technique clear. I leave it to the reader to
generalise such things as comparison function - for now <, or the type
of thing being sorted - for now just any integral types.
The best way to explain is to walk through the code so without further
ado, here goes.

## The Code

Need these for index_sequence etc...

    #include <utility>
    #include <array>

The public interface takes a sequence and we will see that it returns
a sequence. Remember that this is meta programming and really it's all
about the types rather than the values.

    template<typename Int, Int... values> constexpr auto
    sort(std::integer_sequence<Int, values...>);

The pretty interface hides an implementation. This is defined as a
struct. This is just syntactic sugar and saves us having to repeat
some common declarations.

    template<typename Values> struct SortImpl;

Our wrapper function passes on the sequence and calls the implementation.

    template<typename Int, Int... values>
    constexpr auto sort(std::integer_sequence<Int, values...> sequence)
    {
        return SortImpl<decltype(sequence)>::sort();
    }

Now the guts. We use partial specialisation to break out the sequences
of values and indices.

    template<typename Int, Int... values>
    struct SortImpl<std::integer_sequence<Int, values...> >
    {
Create an index corresponding to the positions in the sorted sequence and call an implementation.

        static constexpr auto sort()
        {
            return sort(std::make_index_sequence<sizeof...(values)>{});
        }
A sorted sequence is one where the positions of the elements correspond to the ranking of the elements values. By ranking I mean the order defined by the comparision function (in this case <). In other words position 0 has element with lowest value - rank 0, position 1 has element with rank 1, etc. In general the i'th position contains the i'th ranking element. Here the index parameter pack gives us all the values of i so we can write that in C++ like this:
```
    template<std::size_t... index>
    static constexpr auto sort(std::index_sequence<index...>)
    {
        return std::integer_sequence<Int, ith<index>()...>{};
    }
```

The ith element is the value who's rank is i. We can find this by
looking at all the values and picking out the one with the correct
rank. We have to be a little bit careful though. Repeated values will
lead to ties in ranking. eg for the sequence [1, 2, 2, 3] the ranks
are 1st, 2nd, 2nd, 4th. We can compensate for this by taking into
account the count of each value. Here I'm using a side effect within
the pack expansion to capture the result.


```
    template<std::size_t i>
    static constexpr auto ith()
    {
        Int result{};
        (
            (i >= rankOf<values>() && i < rankOf<values>() + count<values>() ? 
             result = values : Int{}),...);
        return result;
    }
```    

We can define the rank of an element by counting the number of
other elements of lesser value. Note if you we going to
generalise the ordering function this is where you would do it.

```
    template<Int x>
    static constexpr auto rankOf() { return ((x > values) +...); }
```    
The count is similar.
```
    template<Int x>
    static constexpr auto count() { return ((x == values) +...); }
};
```
To show that it works I'm defining equality for integer_sequences. Two
sequences with the same values are equal.
```
template<typename Int, Int... values>
constexpr auto operator==(
    std::integer_sequence<Int, values...>, 
    std::integer_sequence<Int, values...>) { return true; }
```
Sequences with different values are unequal.
```
template<typename Int, Int... values, Int... others>
constexpr auto operator==(
    std::integer_sequence<Int, values...>, 
    std::integer_sequence<Int, others...>) { return false; }

static_assert(
    sort(std::index_sequence<3, 2, 1>{}) == std::index_sequence<1, 2, 3>{});
static_assert(
    sort(std::index_sequence<3, 3, 1>{}) == std::index_sequence<1, 3, 3>{});
```
As an extra check this bit of code converts a sequence to an array.
```
template<typename Int, Int... values>
constexpr auto toArray(std::integer_sequence<Int, values...>)
{
    return std::array<Int, sizeof...(values)>{ values... };
}
```
In godbolt we can see the emitted code is sorted.
```
auto x = toArray(sort(std::index_sequence<3, 2, 1, 9, 42>{}));
```
Is this better than the equivalent recursive definition? I think it's
easier to understand and it's shorter. Is it quicker? Techniqcally it
would be n^3 since for each element we're finding the ith which
involves looking at the rank of each element which requires comparing
each element. But since each of these calculations is a template
instantiation the compiler will cash these intermediate values. I
think it's actually n^2 but with lower overhead than recursive
techniques which are likely to be n log(n).
