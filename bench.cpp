#include <iostream>
#include <random>

#include <absl/container/flat_hash_map.h>
#include <benchmark/benchmark.h>

#include "tuple_map.hpp"

template <typename Distribution>
inline auto n_random_values(size_t N, Distribution distrib) {
  std::random_device rd;
  std::mt19937 gen(rd());

  std::vector<size_t> random_values(N);
  for (auto &x : random_values) {
    x = distrib(gen);
  }
  return random_values;
}

template <class T> T createItem(int32_t a, int32_t b, int32_t c);

template <typename T> inline T createFromRandom(auto distribution, size_t N) {
  auto random_values = n_random_values(N, distribution);
  return T{random_values.begin(), random_values.end()};
}

template <class MapT> auto generate_random_kv_pairs(size_t n) {
  std::vector<typename MapT::value_type> lst;

  auto createNUniformRandomValues = [&n]() {
    return createFromRandom<std::vector<int>>(
        std::uniform_int_distribution<>(1, std::numeric_limits<int>::max()), n);
  };

  auto a = createNUniformRandomValues();
  auto b = createNUniformRandomValues();
  auto c = createNUniformRandomValues();

  for (size_t i = 0; i < n; i++) {
    lst.push_back(createItem<typename MapT::value_type>(a[i], b[i], c[i]));
  }
  return lst;
}

template <class MapT> static void BM_random_access(benchmark::State &state) {
  const size_t N = state.range(0);

  auto random_kv_pairs = generate_random_kv_pairs<MapT>(N);
  auto random_keys = createFromRandom<std::vector<int>>(
      std::uniform_int_distribution<>(1, std::numeric_limits<int>::max()), N);

  auto map = MapT(random_kv_pairs.begin(), random_kv_pairs.end());

  for (auto _ : state) {
    // This code gets timed
    for (size_t i = 0; i < N; i++) {
      auto value = map[random_keys[i]];
      benchmark::DoNotOptimize(std::get<0>(value));
      benchmark::DoNotOptimize(std::get<1>(value));
    }
  }
}

template <class MapT> static void BM_iteration(benchmark::State &state) {
  const size_t N = state.range(0);
  auto random_kv_pairs = generate_random_kv_pairs<MapT>(N);
  auto map = MapT(random_kv_pairs.begin(), random_kv_pairs.end());

  auto values =
      mpl::Overload{[](auto &x)
                      requires requires { x.values(); }
                    {
                      static_assert(std::ranges::range<decltype(x.values())>);
                      return x.values();
                    },
                    [](auto &x) { return x | std::views::values; }};

  for (auto _ : state) {
    for (auto &&value : values(map)) {
      benchmark::DoNotOptimize(std::get<0>(value));
      benchmark::DoNotOptimize(std::get<1>(value));
    }
  }
}

template <typename T> using vector = std::vector<T>;

struct InefficientCacheUsageTypes {
  using Key = int32_t;
  using Tuple = std::tuple<int32_t, int32_t, std::array<char, 56>>;
  template <class K, class V> using flat_hash_map = absl::flat_hash_map<K, V>;
  using TupleMap = swan::tuple_map<Key, flat_hash_map, vector, int32_t, int32_t,
                                   std::array<char, 56>>;
  using SimpleMap = flat_hash_map<Key, Tuple>;
};

struct EfficientCacheUsageTypes {
  using Key = int32_t;
  using Tuple = std::tuple<int32_t, int32_t>;
  template <class K, class V> using flat_hash_map = absl::flat_hash_map<K, V>;
  using TupleMap = swan::tuple_map<Key, flat_hash_map, vector, int32_t, int32_t,
                                   std::array<char, 56>>;
  using SimpleMap = flat_hash_map<Key, Tuple>;
};

template <>
std::pair<const int32_t, InefficientCacheUsageTypes::Tuple>
createItem<std::pair<const int32_t, InefficientCacheUsageTypes::Tuple>>(
    int32_t a, int32_t b, int32_t c) {
  return {a, {b, c, {}}};
}
template <>
std::pair<const int32_t, EfficientCacheUsageTypes::Tuple>
createItem<std::pair<const int32_t, EfficientCacheUsageTypes::Tuple>>(
    int32_t a, int32_t b, int32_t c) {
  return {a, {b, c}};
}

static void BM_SimpleMap_random_access(benchmark::State &state) {
  BM_random_access<InefficientCacheUsageTypes::SimpleMap>(state);
}

static void BM_TupleMap_random_access(benchmark::State &state) {
  BM_random_access<InefficientCacheUsageTypes::TupleMap>(state);
}

static void BM_SimpleMap_iteration(benchmark::State &state) {
  BM_iteration<EfficientCacheUsageTypes::SimpleMap>(state);
}

static void BM_TupleMap_iteration(benchmark::State &state) {
  BM_iteration<EfficientCacheUsageTypes::TupleMap>(state);
}

// Register the function as a benchmark
BENCHMARK(BM_SimpleMap_random_access)->Range(8, 500000);
BENCHMARK(BM_TupleMap_random_access)->Range(8, 500000);

BENCHMARK(BM_SimpleMap_iteration)->Range(8, 500000);
BENCHMARK(BM_TupleMap_iteration)->Range(8, 500000);
// Run the benchmark
BENCHMARK_MAIN();
