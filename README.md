# Hashmap Benchmark 

Benchmarking multiple concurrent hashmap implementations.

## Getting the source
---------------------

This project uses multiple git submodules both for hashmap libraries and various utilities and they need to get initialized.

```
git clone https://github.com/dminikm/HashmapBenchmark.git
git submodule init
```

## Building
-----------

```
mkdir build
cd build
cmake ..
make
make install
```

If you are using mingw on windows (MSYS2/mingw-w64/...), you might need to configure turf to use it's WIN32 implementation to avoid build errors.

```
mkdir build
cd build
cmake cmake -G "MinGW Makefiles" -DTURF_USERCONFIG=turf_options/mingw/turf_userconfig.h.in ..
make
make install
```