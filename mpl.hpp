#pragma once
#include <array>
#include <charconv>
#include <tuple>
#include <type_traits>
#include <utility>

// simple metaprogrammign

namespace mpl {
using std::forward_like;
using std::get;
using std::tuple;

template <class... Ts> struct Overload : Ts... {
  using Ts::operator()...;
};

/// invoke fn(xs)...
template <class Tup, class Fn> void for_each(Tup &&xs, Fn fn) {
  [&]<std::size_t... Is>(std::index_sequence<Is...>) {
    (fn(forward_like<Tup>(get<Is>(xs))), ...);
  }(std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<Tup>>>{});
}

// short-circuit if false
template <class Tup, class Fn> bool all_of(Tup &&xs, Fn fn) {
  return [&]<std::size_t... Is>(std::index_sequence<Is...>) {
    return (... && fn(get<Is>(xs)));
  }(std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<Tup>>>{});
}

/// return tuple(fn(xs)...)
template <class Tup, class Fn> auto transform(Tup &&xs, Fn fn) {
  return [&]<std::size_t... Is>(std::index_sequence<Is...>) {
    auto doOne = [&fn]<class T>(T &&x) -> std::invoke_result_t<Fn, T> {
      return fn(std::forward<T>(x));
    };
    return std::tuple<decltype(doOne(forward_like<Tup>(get<Is>(xs))))...>(
        doOne(forward_like<Tup>(get<Is>(xs)))...);
  }(std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<Tup>>>{});
}

// can zip any number of tuples, but 2 is fine for now.
// return a tuple of forwarding references to inputs
template <class Tup, class Tup2> auto zip(Tup &&x, Tup2 &&y) {
  static_assert(std::tuple_size_v<std::remove_cvref_t<Tup>> ==
                std::tuple_size_v<std::remove_cvref_t<Tup2>>);

  return [&]<std::size_t... Is>(std::index_sequence<Is...>) {
    return tuple(std::forward_as_tuple(forward_like<Tup>(get<Is>(x)),
                                       forward_like<Tup2>(get<Is>(y)))...);
  }(std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<Tup>>>{});
}

} // namespace mpl
