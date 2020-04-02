# prototype
Test projest for Alber Blanc C++ Core

## Комментарии
Переупорядочение полей в структурах не привело к уменьшению их размера из-за выравнивания.
std::string заменил на свою строку, которая содержит до 61 символа не выделяя память на куче.

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

### Custom Small String Optimization (61 chars)
```
./client -z 1 -t 4 -v 0 -l 61 -d 1

ping - pong round trip avg: 12.414 mcs
ping - pong serialization avg: 319 ns
request - reply round trip avg: 12.365 mcs
request - reply serialization avg: 471 ns
```

### Not so small strings (62 chars)
```
./client -z 1 -t 4 -v 0 -l 62 -d 1

ping - pong round trip avg: 13.459 mcs
ping - pong serialization avg: 327 ns
request - reply round trip avg: 13.482 mcs
request - reply serialization avg: 493 ns
```

### Giant strings (100k chars)
```
./client -z 1 -t 4 -v 0 -l 100000 -d 100

ping - pong round trip avg: 349.736 mcs
ping - pong serialization avg: 562 ns
request - reply round trip avg: 379.994 mcs
request - reply serialization avg: 17.139 mcs
```
