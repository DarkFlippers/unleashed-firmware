/**
 * Implementation of compile-time sort for symbol table entries.
 */

#pragma once

#ifdef __cplusplus

#include <iterator>
#include <array>

namespace cstd {

template <typename RAIt>
constexpr RAIt next(RAIt it, typename std::iterator_traits<RAIt>::difference_type n = 1) {
    return it + n;
}

template <typename RAIt>
constexpr auto distance(RAIt first, RAIt last) {
    return last - first;
}

template <class ForwardIt1, class ForwardIt2>
constexpr void iter_swap(ForwardIt1 a, ForwardIt2 b) {
    auto temp = std::move(*a);
    *a = std::move(*b);
    *b = std::move(temp);
}

template <class InputIt, class UnaryPredicate>
constexpr InputIt find_if_not(InputIt first, InputIt last, UnaryPredicate q) {
    for(; first != last; ++first) {
        if(!q(*first)) {
            return first;
        }
    }
    return last;
}

template <class ForwardIt, class UnaryPredicate>
constexpr ForwardIt partition(ForwardIt first, ForwardIt last, UnaryPredicate p) {
    first = cstd::find_if_not(first, last, p);
    if(first == last) return first;

    for(ForwardIt i = cstd::next(first); i != last; ++i) {
        if(p(*i)) {
            cstd::iter_swap(i, first);
            ++first;
        }
    }
    return first;
}

}

template <class RAIt, class Compare = std::less<> >
constexpr void quick_sort(RAIt first, RAIt last, Compare cmp = Compare{}) {
    auto const N = cstd::distance(first, last);
    if(N <= 1) return;
    auto const pivot = *cstd::next(first, N / 2);
    auto const middle1 =
        cstd::partition(first, last, [=](auto const& elem) { return cmp(elem, pivot); });
    auto const middle2 =
        cstd::partition(middle1, last, [=](auto const& elem) { return !cmp(pivot, elem); });
    quick_sort(first, middle1, cmp); // assert(std::is_sorted(first, middle1, cmp));
    quick_sort(middle2, last, cmp); // assert(std::is_sorted(middle2, last, cmp));
}

template <typename Range>
constexpr auto sort(Range&& range) {
    quick_sort(std::begin(range), std::end(range));
    return range;
}

template <typename V, typename... T>
constexpr auto array_of(T&&... t) -> std::array<V, sizeof...(T)> {
    return {{std::forward<T>(t)...}};
}

template <typename T, typename... N>
constexpr auto my_make_array(N&&... args) -> std::array<T, sizeof...(args)> {
    return {std::forward<N>(args)...};
}

namespace traits {
template <typename T, typename... Ts>
struct array_type {
    using type = T;
};

template <typename T, typename... Ts>
static constexpr bool are_same_type() {
    return std::conjunction_v<std::is_same<T, Ts>...>;
}

}

template <typename... T>
constexpr auto create_array(const T&&... values) {
    using array_type = typename traits::array_type<T...>::type;
    static_assert(sizeof...(T) > 0, "an array must have at least one element");
    static_assert(traits::are_same_type<T...>(), "all elements must have same type");
    return std::array<array_type, sizeof...(T)>{values...};
}

template <typename T, typename... Ts>
constexpr auto create_array_t(const Ts&&... values) {
    using array_type = T;
    static_assert(sizeof...(Ts) > 0, "an array must have at least one element");
    static_assert(traits::are_same_type<Ts...>(), "all elements must have same type");
    return std::array<array_type, sizeof...(Ts)>{static_cast<T>(values)...};
}

#endif
