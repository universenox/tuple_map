#include "mpl.hpp"
#include <fmt/ranges.h>

#include <boost/ut.hpp>
#include <set>
#include <type_traits>
using namespace boost::ut;

suite<"mpl"> mpl_test = [] {
  using boost::ut::operator""_test;
  "for_each"_test = [] {
    std::tuple<int, double> x{1, 2.3};

    size_t i = 0;
    mpl::for_each(x, [&i](auto &y) {
      if (i == 0)
        expect(y == 1);
      else
        expect(y == 2.3);
      i++;
    });
  };

  "all_of-short_circuits"_test = [] {
    std::tuple<int, int> x{1, 2};
    std::set<int> seen;
    mpl::all_of(x, [&seen](int &y) {
      seen.insert(y);
      return y == 2;
    });
    expect(seen.size() == 1);
  };

  "transform"_test = [] {
    std::tuple<int, int> a{1, 2};
    auto x = mpl::transform(a, [](int &x) -> int { return 2 * x; });
    auto y = mpl::transform(a, [](int &x) -> int & { return x; });

    static_assert(std::is_lvalue_reference_v<decltype(std::get<0>(y))>);
    static_assert(std::is_lvalue_reference_v<decltype(std::get<1>(y))>);
    expect(std::get<0>(y) == 1);
    expect(std::get<1>(y) == 2);

    expect(std::get<0>(x) == 2);
    expect(std::get<1>(x) == 4);
  };

  "zip"_test = [] {
    std::tuple<int, double> a{1, 2.2};
    std::tuple<double, int> b{3.3, 5};

    // lvalues
    auto x = mpl::zip(a, b);
    static_assert(std::is_lvalue_reference_v<decltype(std::get<0>(x))>);
    static_assert(std::is_lvalue_reference_v<decltype(std::get<1>(x))>);

    expect(std::get<0>(x) == std::tuple{1, 3.3});
    expect(std::get<1>(x) == std::tuple{2.2, 5});

    auto y = mpl::zip(std::tuple(1, 2.2), std::tuple(3.3, 5));
    expect(std::get<0>(y) == std::tuple{1, 3.3});
    expect(std::get<1>(y) == std::tuple{2.2, 5});
  };
};
