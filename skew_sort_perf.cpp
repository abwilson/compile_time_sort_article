#include <type_traits>
#include <utility>

#include DATA_FILE

template <class T, T... s>
using iseq = std::integer_sequence<T, s...>;

template <class T, T v>
using ic = std::integral_constant<T, v>;

template <class T, T v1, T v2>
constexpr auto ic_less_impl(ic<T, v1>, ic<T, v2>) -> ic<bool, v1 < v2>;
template <class ic1, class ic2>
using ic_less = decltype(ic_less_impl(ic1(), ic2()));

template <bool b>
using bool_cond_t = std::conditional_t<b, std::true_type, std::false_type>;

struct nil {};

template <class T, T v, T... s>
constexpr auto iseq_front_impl(iseq<T, v, s...>) -> ic<T, v>;
template <class T>
constexpr auto iseq_front_impl(iseq<T>) -> nil;
template <class seq>
using iseq_front = decltype(iseq_front_impl(seq()));

template <class T, T v, T... s>
constexpr auto iseq_pop_front_impl(iseq<T, v, s...>) -> iseq<T, s...>;
template <class seq>
using iseq_pop_front = decltype(iseq_pop_front_impl(seq()));

template <class T, T v, T... s>
constexpr auto iseq_append_impl(iseq<T, s...>, ic<T, v>) -> iseq<T, s..., v>;
template <class T, T v>
constexpr auto iseq_append_impl(nil, ic<T, v>) -> iseq<T, v>;
template <class seq, class c>
using iseq_append = decltype(iseq_append_impl(seq(), c()));

template <class seq>
using iseq_is_empty = bool_cond_t<std::is_same<iseq_front<seq>, nil>::value>;

template <class X, class L, class R>
struct skew_heap {};

template <class X, class L, class R>
constexpr auto skh_get_top_impl(skew_heap<X, L, R>) -> X;
template <class H>
using skh_get_top = decltype(skh_get_top_impl(H()));

template <class X, class L, class R>
constexpr auto skh_get_left_impl(skew_heap<X, L, R>) -> L;
template <class H>
using skh_get_left = decltype(skh_get_left_impl(H()));

template <class X, class L, class R>
constexpr auto skh_get_right_impl(skew_heap<X, L, R>) -> R;
template <class H>
using skh_get_right = decltype(skh_get_right_impl(H()));

template <class H>
using skh_is_empty = bool_cond_t<std::is_same<H, nil>::value>;

template <class H1, class H2>
constexpr auto skh_merge_impl(H1, H2) -> decltype(auto) {
    if constexpr (skh_is_empty<H1>::value) {
        return H2{};
    } else if constexpr (skh_is_empty<H2>::value) {
        return H1{};
    } else {
        using x1 = skh_get_top<H1>;
        using l1 = skh_get_left<H1>;
        using r1 = skh_get_right<H1>;

        using x2 = skh_get_top<H2>;
        using l2 = skh_get_left<H2>;
        using r2 = skh_get_right<H2>;

        if constexpr (ic_less<x2, x1>::value) {
            using new_r2 = decltype(skh_merge_impl(H1(), r2()));
            return skew_heap<x2, new_r2, l2> {};
        } else {
            using new_r1 = decltype(skh_merge_impl(r1(), H2()));
            return skew_heap<x1, new_r1, l1>{};
        }
    }
}
template <class H1, class H2>
using skh_merge = decltype(skh_merge_impl(H1(), H2()));

template <class H1, class IC1>
using skh_push = skh_merge<H1, skew_heap<IC1, nil, nil>>;

template <class H>
using skh_pop = skh_merge<skh_get_left<H>, skh_get_right<H>>;

template <class H, class seq>
constexpr auto skh_heapify_impl(H, seq) -> decltype(auto) {
    if constexpr (iseq_is_empty<seq>::value) {
        return H{};
    } else {
        using val = iseq_front<seq>;
        return skh_heapify_impl(skh_push<H, val>{}, iseq_pop_front<seq>{});
    }
}
template <class seq>
using skh_heapify = decltype(skh_heapify_impl(nil(), seq()));

template <class H, class seq>
constexpr auto skh_to_sortseq_impl(H, seq) -> decltype(auto) {
    if constexpr (skh_is_empty<H>::value) {
        return seq{};
    } else {
        using val = skh_get_top<H>;
        return skh_to_sortseq_impl(skh_pop<H>{}, iseq_append<seq, val>{});
    }
}
template <class H>
using skh_to_sortseq = decltype(skh_to_sortseq_impl(H(), nil()));

template <class seq>
using sort_seq = skh_to_sortseq<skh_heapify<seq>>;

auto x = sort_seq<iseq<int, DATA> >{};

int main(){}
