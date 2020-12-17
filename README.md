# Hashmap Benchmark 

Benchmarking multiple concurrent hashmap implementations.

## Getting the source

This project uses multiple git submodules both for hashmap libraries and various utilities and they need to get initialized.

```
git clone https://github.com/dminikm/HashmapBenchmark.git
git submodule init
git submodule update
```

## Building

```shell
mkdir build
cd build
cmake ..
cmake --build . --config Release --target install
```

If you are using mingw on windows (MSYS2/mingw-w64/...), you might need to configure turf to use it's WIN32 implementation to avoid build errors.

```shell
mkdir build
cd build
cmake -G "MinGW Makefiles" -DTURF_USERCONFIG=turf_options/mingw/turf_userconfig.h.in -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS=-m64 ..
cmake --build . --config Release --target install
```

## Running

### Wordcount test
Wordcount has 4 implementations:
 - libcuckoo
 - tbb-hash - tbb::concurrent_hash_map
 - tbb-unordered - tbb::concurrent_unordered_map + tbb::atomic
 - std-blocking - std::unordered_map + std::mutex

### Example
Running libcuckoo benchmark with 4 threads, 60 runs outputting results to json:
```shell
mkdir runs
./HashmapBenchmark wordcount -t 4 -r 60 --implementation=libcuckoo --json=runs/4t_60r_libcuckoo.json --dataset=../data/test.ft.txt.out
```