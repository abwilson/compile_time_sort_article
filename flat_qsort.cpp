#include "flat_qsort.h"

struct SimpleTraits
{
    template<typename T>
    static constexpr auto value(Tag<T>){ return T::value; }
    static constexpr std::less<> compare{};
} traits;


static_assert(grade(traits, TypeList{}) == TypeList{}, "");
static_assert(
    grade(traits, TypeList<Int<1> >{}) == TypeList<TypeList<Int<1> > >{}, "");

static_assert(
    grade(traits, TypeList<Int<1>, Int<2> >{}) ==
    TypeList<TypeList<Int<1> >, TypeList<Int<2> > >{}, "");

static_assert(
    grade(traits, TypeList<Int<2>, Int<1> >{}) ==
    TypeList<TypeList<Int<1> >, TypeList<Int<2> > >{}, "");

static_assert(
    std::is_same_v<
    decltype(grade(traits, TypeList<Int<2>, Int<1>, Int<3> >{})),
    TypeList<TypeList<Int<1> >, TypeList<Int<2> >, TypeList<Int<3> > > >, "");


static_assert(
    std::is_same_v<
    decltype(flatQsort(traits, TypeList{})), TypeList<> >, "");

static_assert(
    std::is_same_v<
        decltype(flatQsort(traits, TypeList<Int<1> >{})),
    TypeList<Int<1> > >, "");

static_assert(
    std::is_same_v<
    decltype(flatQsort(traits, TypeList<Int<1>, Int<2> >{})),
    TypeList<Int<1>, Int<2> > >, "");

static_assert(
    std::is_same_v<
    decltype(flatQsort(traits, TypeList<Int<2>, Int<1> >{})),
    TypeList<Int<1>, Int<2> > >, "");

static_assert(
    std::is_same_v<
    decltype(flatQsort(traits, TypeList<Int<2>, Int<3>, Int<1> >{})),
    TypeList<Int<1>, Int<2>, Int<3> > >, "");

static_assert(
    std::is_same_v<
    decltype(flatQsort(traits, TypeList<Int<2>, Int<3>, Int<4>, Int<1> >{})),
    TypeList<Int<1>, Int<2>, Int<3>, Int<4> > >, "");

static_assert(
    std::is_same_v<
    decltype(flatQsort(traits, TypeList<Int<1>, Int<1>, Int<1>, Int<1> >{})),
    TypeList<Int<1>, Int<1>, Int<1>, Int<1> > >, "");

int main(){}
