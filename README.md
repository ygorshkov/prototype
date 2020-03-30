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
./client -z 1 -t 4 -v 0 -d 1

ping - pong round trip count: 500000
ping - pong round trip avg: 12.111 mcs
ping - pong round trip total: 6.055 s
ping - pong serialization count: 999192
ping - pong serialization avg: 321 ns
ping - pong serialization total: 321.208 ms
request - reply round trip count: 499999
request - reply round trip avg: 12.013 mcs
request - reply round trip total: 6.006 s
request - reply serialization count: 999219
request - reply serialization avg: 480 ns
request - reply serialization total: 479.772 ms
```

### Not so small strings (23 chars for libc++)
```
ping - pong round trip count: 499999
ping - pong round trip avg: 13.296 mcs
ping - pong round trip total: 6.648 s
ping - pong serialization count: 999339
ping - pong serialization avg: 328 ns
ping - pong serialization total: 328.116 ms
request - reply round trip count: 499999
request - reply round trip avg: 13.457 mcs
request - reply round trip total: 6.728 s
request - reply serialization count: 999397
request - reply serialization avg: 678 ns
request - reply serialization total: 677.720 ms
```

### Giant strings (100k chars)
```
./client -z 1 -t 4 -v 0 -d 100
ping - pong round trip count: 4998
ping - pong round trip avg: 191.959 mcs
ping - pong round trip total: 959.415 ms
ping - pong serialization count: 9998
ping - pong serialization avg: 486 ns
ping - pong serialization total: 4.867 ms
request - reply round trip count: 4997
request - reply round trip avg: 396.038 mcs
request - reply round trip total: 1.979 s
request - reply serialization count: 9997
request - reply serialization avg: 20.982 mcs
request - reply serialization total: 209.766 ms

```
