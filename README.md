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

ping - pong round trip avg: 12.414 mcs
ping - pong serialization avg: 319 ns
request - reply round trip avg: 12.365 mcs
request - reply serialization avg: 471 ns
```

### Not so small strings (23 chars for libc++)
```
./client -z 1 -t 4 -v 0 -d 1

ping - pong round trip avg: 12.749 mcs
ping - pong serialization avg: 328 ns
request - reply round trip avg: 13.257 mcs
request - reply serialization avg: 698 ns
```

### Giant strings (100k chars)
```
./client -z 1 -t 4 -v 0 -d 100

ping - pong round trip avg: 223.283 mcs
ping - pong serialization avg: 462 ns
request - reply round trip avg: 385.310 mcs
request - reply serialization avg: 20.960 mcs
```
