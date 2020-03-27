# prototype
Test projest for Alber Blanc C++ Core

## Комментарии
Переупорядочение полей в структурах не привело к уменьшению их размера из-за выравнивания.
Заменять std::string не стал, так как эффект от эксплуатации Small String Optimization дал
лишь ~5% ускорения.

## Description
I've been using [boost::pfr](https://github.com/apolukhin/magic_get) and
[YetAnotherSerializer](https://github.com/niXman/yas) for serialization.
And [CppServer](https://github.com/chronoxor/CppServer) as network lib.

## Build
### CppServer (should be git submodule)
```bash
cd prototype/lib/cppserver
gil update
cd build
OPENSSL_ROOT_DIR=/usr/local/opt/openssl/ ./unix.sh
```

### prototype
```bash
cd ../../../..
mkdir build
cd build

OPENSSL_ROOT_DIR=/usr/local/opt/openssl/ cmake -DCMAKE_BUILD_TYPE=Release ../prototype
cmake --build .
```

## Run
```bash
bin/server
bin/client -z 10 -t 1 -v 0
```

## Some results:

### Small String Optimization (22 chars for libc++)
```
ping - pong round trip count: 124720
ping - pong round trip avg: 38.994 mcs
ping - pong round trip total: 4.863 s
ping - pong serialization count: 249440
ping - pong serialization avg: 202 ns
ping - pong serialization total: 50.441 ms
request - reply round trip count: 124719
request - reply round trip avg: 40.015 mcs
request - reply round trip total: 4.990 s
request - reply serialization count: 249439
request - reply serialization avg: 408 ns
request - reply serialization total: 102.020 ms
```

### Not so small strings (23 chars for libc++)
```
ping - pong round trip count: 115975
ping - pong round trip avg: 41.000 mcs
ping - pong round trip total: 4.755 s
ping - pong serialization count: 231950
ping - pong serialization avg: 172 ns
ping - pong serialization total: 40.081 ms
request - reply round trip count: 115974
request - reply round trip avg: 44.104 mcs
request - reply round trip total: 5.114 s
request - reply serialization count: 231949
request - reply serialization avg: 803 ns
request - reply serialization total: 186.330 ms
```

#### another launch
```
ping - pong round trip count: 121321
ping - pong round trip avg: 39.211 mcs
ping - pong round trip total: 4.757 s
ping - pong serialization count: 242642
ping - pong serialization avg: 163 ns
ping - pong serialization total: 39.775 ms
request - reply round trip count: 121320
request - reply round trip avg: 42.111 mcs
request - reply round trip total: 5.108 s
request - reply serialization count: 242641
request - reply serialization avg: 782 ns
request - reply serialization total: 189.900 ms
```

### Giant strings (100k chars)
```
ping - pong round trip count: 10941
ping - pong round trip avg: 63.100 mcs
ping - pong round trip total: 690.385 ms
ping - pong serialization count: 21883
ping - pong serialization avg: 406 ns
ping - pong serialization total: 8.891 ms
request - reply round trip count: 10941
request - reply round trip avg: 844.624 mcs
request - reply round trip total: 9.241 s
request - reply serialization count: 21882
request - reply serialization avg: 22.590 mcs
request - reply serialization total: 494.334 ms
```
