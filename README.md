# Chadpp

A discord bot made with c++

This project is just made for fun.

## Building
Linux only.
First, install dependecies below.

Then clone this repo.
```
    git clone https://github.com/DK0280705/chadpp.git
    cd chadpp
    git submodule update --init --recursive
```
Create `config.json` file with your favorite editor.
```json
{
    "token": "Your token",
    "botlog_id": 123456789098765432,
    "guild_id": 123456789098765432,
    "owner_id": 123456789098765432,
    "conn_string": "Your PostgreSQL connection string"
}
```

Then configure cmake, you can use GNU make if you want.
```
    cmake -DCMAKE_BUILD_TYPE=Release -GNinja -B ./build
```
Compile with:
```
    cmake --build ./build -jN
```
Replace N with your number of processor cores.

Then run your bot:
```
    cd build
    ./chadpp
```

## External Dependencies
- [LAME](https://lame.sourceforge.io)
- [Sodium](https://github.com/jedisct1/libsodium)
- [Opus](https://www.opus-codec.org)
- [OpenSSL](https://openssl.org)
- [Zlib](https://zlib.net)
- [PQXX](https://github.com/jtv/libpqxx)

## Included Dependencies
No need to install these as they're already included in this project.
- [DPP](https://github.com/brainboxdotcc/DPP) (Using my [fork](https://github.com/DK0280705/DPP))
- [Nlohmann JSON](https://github.com/nlohmann/json)
- [`{fmt}`](https://github.com/fmtlib/fmt)