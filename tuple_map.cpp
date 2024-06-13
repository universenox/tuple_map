#include <array>
#include <iostream>
#include <map>
#include <tuple>

#include "tuple_map.hpp"
#include <fmt/ranges.h>

template <typename K, typename V> using Map = std::map<K, V>;

#include <boost/ut.hpp>
using namespace boost::ut;

template <typename T> using vector = std::vector<T>;

using T = swan::tuple_map<int, Map, vector, int>;

static_assert(std::ranges::range<swan::tuple_map<int, Map, vector, int>>);

suite<"tuple_map"> tuple_map_test = [] {
  "emplace"_test = [] {
    swan::tuple_map<int, Map, vector, int, int> map;
    map.emplace(2, 1, 3);
    expect(map[2] == std::tuple<int, int>(1, 3));
  };

  "construct"_test = [] {
    swan::tuple_map<int, Map, vector, vector<int>, std::tuple<int, double>, int>
        map{{
            {2, {{}, {1, 2.0}, 1}},  //
            {1, {{}, {3, 5.0}, 100}} //
        }};
    expect(map[2] == std::tuple<std::vector<int>, //
                                std::tuple<int, double>, int>({}, {1, 2.0}, 1));
    expect(map[1] ==
           std::tuple<std::vector<int>, //
                      std::tuple<int, double>, int>({}, {3, 5.0}, 100));
  };
};
