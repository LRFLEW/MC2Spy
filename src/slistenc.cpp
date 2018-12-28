/*
 * MC2Spy - Implementation of the GameSpy Protocol for Midnight Club 2
 * Copyright (C) 2018  LRFLEW
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "slistenc.hpp"

#include <numeric>
#include <random>
#include <utility>

constexpr boost::string_view game_key = "y6E3c9"_sv;

static std::default_random_engine rng(std::random_device{}());

template<std::size_t N>
static std::array<std::uint8_t, N> random_array() {
    std::array<std::uint8_t, N> arr;
    std::uniform_int_distribution<> dist(0, 255);
    std::generate(arr.begin(), arr.end(),
        [&dist]() -> std::uint8_t { return dist(rng); });
    return arr;
}

static std::size_t init_sub(const std::array<std::uint8_t, 256> &lookup, std::size_t index,
    const std::array<std::uint8_t, 8> &clientkey, std::size_t &n1, std::size_t &n2) {

    // Fast compute smallest 2^n - 1 >= index, assuming index < 256
    std::size_t mask = index;
    mask |= mask >> 1; mask |= mask >> 2; mask |= mask >> 4;

    std::size_t result;
    for (std::size_t i = 0; i < 12; ++i) {
        n1 = lookup[n1 & 0xFF] + clientkey[n2];
        if (++n2 >= clientkey.size()) {
            n2 = 0;
            n1 += clientkey.size();
        }
        result = n1 & mask;
        if (result <= index) return result;
    }

    // If sample-rejection causes too many rejection, use fallback
    return result % index;
}

void slistenc::init(std::array<std::uint8_t, 8> clientkey) {
    override_enc = true;

    std::array<std::uint8_t, 8> crypt = random_array<8>(); // Unused?
    std::array<std::uint8_t, 25> serverkey = random_array<25>();

    this->uint8(static_cast<std::uint8_t>(2 + crypt.size()) ^ 0xEC);
    this->be_uint16(0); // backend flags
    this->array(crypt);
    this->uint8(static_cast<std::uint8_t>(serverkey.size()) ^ 0xEA);
    this->array(serverkey);

    // modify clientkey based on serverkey and gamekey for the final key
    for (std::size_t i = 0; i < serverkey.size(); ++i) {
        clientkey[(game_key[i % game_key.size()] * i) % clientkey.size()] ^=
            clientkey[i % clientkey.size()] ^ serverkey[i];
    }

    std::iota(lookup.begin(), lookup.end(), 0);
    std::size_t n1 = 0, n2 = 0;
    for (std::size_t i = 255; i > 0; --i) {
        std::swap(lookup[i], lookup[init_sub(lookup, i, clientkey, n1, n2)]);
    }
    a = lookup[1]; b = lookup[3]; c = lookup[5]; d = lookup[7]; e = lookup[n1 & 0xFF];

    override_enc = false;
}

void slistenc::write(const void *data, std::size_t size) {
    std::size_t off = _data.size();
    _data.resize(off + size);
    const std::uint8_t *ptr = reinterpret_cast<const std::uint8_t *>(data);
    if (override_enc) std::copy_n(ptr, size, _data.begin() + off);
    else std::transform(ptr, ptr + size, _data.begin() + off, boost::bind(&slistenc::enc, this, _1));
}

std::uint8_t slistenc::enc(std::uint8_t v) {
    b += lookup[a++];
    std::uint8_t z = lookup[e];
    lookup[e] = lookup[b];
    lookup[b] = lookup[d];
    lookup[d] = lookup[a];
    lookup[a] = z;
    c += lookup[z];

    e = lookup[lookup[(lookup[b] + lookup[d] + lookup[e]) & 0xFF]] ^
        lookup[(lookup[a] + lookup[c]) & 0xFF] ^ v;

    d = v;
    return e;
}
