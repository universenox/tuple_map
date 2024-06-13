# Tuple Map
Logically, this is a map from a key to a tuple of values. However, we store
each separate field in its own std::vector, and keep a map of Key to vector index.

Note that I perform benchmarks against abseil::flat_hash_map, which is known to
be a rather performant implementation.

I compare `absl::flat_hash_map<Key, std::tuple<Vs...>` to 
`tuple_map<Key, absl::flat_hash_map, std::vector, Vs...>`.

I will warn that this implementation is rather incomplete.

## Using this
If you check out this repo and have the `nix` package manager installed, 
to run benchmarks and test is as easy as `nix build` then 
`./result/bench` and `./result/test`.

## Drawbacks
- append-only
- missing several standard functions
- ...

## Advantages
### Cache Efficiency
Where accessing the most common fields would otherwise pollute the cache with
fields that are not so interesting.

```cpp
using Tuple = std::tuple<int32_t, int32_t, std::array<char, 56>>;
```

A typical block size is 64B. But suppose we only care about the first two
felds, the first 8B. When we access the first two values, assuming Tuple is
64B aligned, we will pull in 56B of garbage into our cache.

With tuple_map, however, each field gets its own vector. Accessing a 
single integer will hence pull in 7 others of its kind.

I performed a benchmark where I randomly generated N values, 
and access the two integers for N random keys.

```
----------------------------------------------------------------------------
Benchmark                                  Time             CPU   Iterations
----------------------------------------------------------------------------
BM_SimpleMap_random_access/8            32.5 ns         32.4 ns     21249691
BM_SimpleMap_random_access/64            267 ns          267 ns      2576861
BM_SimpleMap_random_access/512          2849 ns         2836 ns       251688
BM_SimpleMap_random_access/4096        30871 ns        30761 ns        19821
BM_SimpleMap_random_access/32768      395136 ns       393157 ns         1595
BM_SimpleMap_random_access/262144    8862904 ns      8818139 ns           63
BM_SimpleMap_random_access/500000   17541189 ns     17470491 ns           39

BM_TupleMap_random_access/8             24.1 ns         24.1 ns     28923661
BM_TupleMap_random_access/64             208 ns          207 ns      3580036
BM_TupleMap_random_access/512           1623 ns         1620 ns       441842
BM_TupleMap_random_access/4096         21105 ns        20972 ns        33692
BM_TupleMap_random_access/32768       274816 ns       273275 ns         2963
BM_TupleMap_random_access/262144     5447535 ns      5410695 ns          105
BM_TupleMap_random_access/500000    12668433 ns     12587823 ns           62
```

### Value Iteration
Iterating over the values should be faster as you'll be iterating over vectors.

Example:
We run the same benchmark as above, but with
```cpp
using Tuple = std::tuple<int32_t, int32_t>;
```
Here, we should not be receiving any benefit from the cache stuff like above. 

```
BM_SimpleMap_iteration/8                9.92 ns         9.86 ns    125985394
BM_SimpleMap_iteration/64                107 ns          107 ns      6811450
BM_SimpleMap_iteration/512               924 ns          923 ns       717827
BM_SimpleMap_iteration/4096             9152 ns         9135 ns        77143
BM_SimpleMap_iteration/32768          140368 ns       140128 ns         5068
BM_SimpleMap_iteration/262144        1149680 ns      1147497 ns          607
BM_SimpleMap_iteration/500000        2297087 ns      2292452 ns          302

BM_TupleMap_iteration/8                 4.48 ns         4.47 ns    157343951
BM_TupleMap_iteration/64                31.9 ns         31.8 ns     22008650
BM_TupleMap_iteration/512                256 ns          255 ns      2725282
BM_TupleMap_iteration/4096              2016 ns         2013 ns       347187
BM_TupleMap_iteration/32768            18543 ns        18443 ns        38854
BM_TupleMap_iteration/262144          156438 ns       155556 ns         4254
BM_TupleMap_iteration/500000          253405 ns       252782 ns         2727
```

