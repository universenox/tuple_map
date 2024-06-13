#pragma once

#include <cassert>
#include <initializer_list>
#include <ranges>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include <fmt/ranges.h>

#include "mpl.hpp"

namespace swan {

template <class Key, template <class, class> class Map,
          template <class> class Vector, class... Vs>
struct tuple_map {
public:
  using key_type = Key;
  using mapped_type = std::tuple<Vs...>;

  using value_type = std::pair<const Key, mapped_type>;
  using reference = std::tuple<Vs &...>;
  using const_reference = std::tuple<const Vs &...>;

  template <bool isConst> struct Iterator;
  using iterator = Iterator<false>;
  using const_iterator = Iterator<true>;

  template <bool isConst> struct ValueIterator;
  using value_iterator = ValueIterator<false>;
  using const_value_iterator = ValueIterator<true>;

private:
  using IndexMap = Map<Key, size_t>;

  // maps key to corresponding index in std::get<Is>(_values)
  IndexMap _map_to_idx;
  std::tuple<Vector<Vs>...> _values;

public:
  // ---------------------- Non-standard public --------------------------

  // prefer this to std::views::values; skip using map.
  std::ranges::subrange<value_iterator, value_iterator,
                        std::ranges::subrange_kind::sized>
  values();

  std::ranges::subrange<const_value_iterator, const_value_iterator,
                        std::ranges::subrange_kind::sized>
  values() const;

  // ---------------------- Constructors -----------------------------------

  constexpr tuple_map() = default;

  template <class InputIt> constexpr tuple_map(InputIt begin, InputIt end);
  constexpr tuple_map(std::initializer_list<value_type> il);

  template <std::ranges::input_range R>
  constexpr tuple_map(R &&range)
    requires(std::same_as<value_type, std::ranges::range_value_t<R>>);

  constexpr tuple_map(const tuple_map &other) = default;
  constexpr tuple_map(tuple_map &&other) = default;
  constexpr tuple_map &operator=(const tuple_map &other) = default;
  constexpr tuple_map &operator=(tuple_map &&other) = default;
  ~tuple_map() = default;
  // ----------------------------------------------------------------------

  constexpr reference operator[](Key key);
  constexpr iterator begin();
  constexpr iterator end();
  constexpr const_iterator begin() const;
  constexpr const_iterator end() const;
  constexpr size_t size() const;

  template <class K2, class... Us>
  constexpr void emplace(K2 &&key, Us &&...value);

private:
  reference value_refs(size_t idx);
  const_reference value_refs(size_t idx) const;
};

// ----------------------  implementation -----------------------------

#define TS                                                                     \
  template <class Key, template <class, class> class Map,                      \
            template <class> class Vector, class... Vs>
#define MAP tuple_map<Key, Map, Vector, Vs...>

TS template <std::ranges::input_range R>
constexpr MAP::tuple_map(R &&range)
  requires(std::same_as<value_type, std::ranges::range_value_t<R>>)
    : tuple_map(std::ranges::begin(range), std::ranges::end(range)) {}

TS template <class InputIt>
constexpr MAP::tuple_map(InputIt begin, InputIt end) {
  for (auto &&v : std::ranges::subrange(begin, end)) {
    std::apply(
        [&]<typename... Ts>(Ts &&...rest) {
          emplace(std::forward<decltype(v.first)>(v.first),
                  std::forward<Ts>(rest)...);
        },
        v.second);
  }
}

TS constexpr MAP::tuple_map(std::initializer_list<value_type> il)
    : tuple_map(il.begin(), il.end()) {}

TS constexpr MAP::reference MAP::operator[](Key key) {
  auto idx = _map_to_idx[key];
  return value_refs(idx);
}

TS constexpr size_t MAP::size() const { return _map_to_idx.size(); };

TS template <bool isConst> struct MAP::Iterator {
  using difference_type = int;

  using MapIterator =
      std::conditional_t<isConst, typename IndexMap::const_iterator,
                         typename IndexMap::iterator>;
  using Parent = std::conditional_t<isConst, const tuple_map, tuple_map>;

  MapIterator _map_it;
  Parent *_parent;

  Iterator &operator++() {
    _map_it++;
    return *this;
  }
  Iterator operator++(int) {
    Iterator retval = *this;
    ++(*this);
    return retval;
  }
  auto operator<=>(const Iterator &other) const {
    assert(_parent == other._parent);
    return _map_it <=> other._map_it;
  }
  auto operator==(const Iterator &other) const {
    assert(_parent == other._parent);
    return _map_it == other._map_it;
  }
  auto operator*() {
    auto idx = _map_it->second;
    return std::pair(_map_it->first, _parent->value_refs(idx));
  }
  auto operator*() const {
    auto idx = _map_it->second;
    return std::pair(_map_it->first, _parent->value_refs(idx));
  }
};

TS constexpr MAP::iterator MAP::begin() {
  return typename MAP::iterator{_map_to_idx.begin(), this};
}
TS constexpr MAP::iterator MAP::end() {
  return iterator{_map_to_idx.end(), this};
}
TS constexpr MAP::const_iterator MAP::begin() const {
  return const_iterator{_map_to_idx.begin(), this};
}
TS constexpr MAP::const_iterator MAP::end() const {
  return const_iterator{_map_to_idx.end(), this};
}

TS template <class K2, class... Us>
constexpr void MAP::emplace(K2 &&key, Us &&...value) {
  static_assert(sizeof...(value) == sizeof...(Vs),
                "use forward_as_tuple for multiple constructor args per value");

  // This is why we cannot erase (for now)
  // Could consider allowing different strategies for assigning / removing
  // values at indices
  _map_to_idx.insert({std::forward<K2>(key), size()});

  // We place one value into each _values
  mpl::for_each(
      mpl::zip(_values, std::forward_as_tuple(value...)),
      []<class T>(T &&y) { std::get<0>(y).emplace_back(std::get<1>(y)); });
}

TS template <bool isConst> struct MAP::ValueIterator {
  using difference_type = int;
  using Parent = std::conditional_t<isConst, const tuple_map, tuple_map>;
  using reference =
      std::conditional_t<isConst, MAP::const_reference, MAP::reference>;

  size_t _idx;
  Parent *_parent;

  ValueIterator &operator++() {
    _idx++;
    return *this;
  }
  ValueIterator operator++(int) {
    ValueIterator retval = *this;
    ++(*this);
    return retval;
  }
  auto operator<=>(const ValueIterator &other) const {
    assert(_parent == other._parent);
    return _idx <=> other._idx;
  }
  auto operator==(const ValueIterator &other) const {
    assert(_parent == other._parent);
    return _idx == other._idx;
  }
  int operator-(const ValueIterator &other) const {
    assert(_parent == other._parent);
    auto x = *this;
    x._idx -= other._idx;
    return x;
  }
  reference operator*() { return _parent->value_refs(_idx); }
  const_reference operator*() const { return _parent->value_refs(_idx); }
};

TS std::ranges::subrange<class MAP::template ValueIterator<false>,
                         class MAP::template ValueIterator<false>,
                         std::ranges::subrange_kind::sized>
MAP::values() {
  return std::ranges::subrange(ValueIterator<false>{0, this},
                               ValueIterator<false>{size(), this});
}

TS std::ranges::subrange<class MAP::template ValueIterator<true>,
                         class MAP::template ValueIterator<true>,
                         std::ranges::subrange_kind::sized>
MAP::values() const {
  return std::ranges::subrange(ValueIterator<true>{0, this},
                               ValueIterator<true>{size(), this});
}

TS MAP::reference MAP::value_refs(size_t idx) {
  return mpl::transform(_values,
                        [&idx]<class T>(T &vec) ->
                        typename T::value_type & { return vec[idx]; });
}

TS MAP::const_reference MAP::value_refs(size_t idx) const {
  return mpl::transform(
      _values, [&idx]<class T>(T &vec) -> const typename T::value_type & {
        return vec[idx];
      });
}

#undef TS
#undef MAP

} // namespace swan
