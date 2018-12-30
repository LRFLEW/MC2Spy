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

#include "qrenc.hpp"

#include <numeric>

constexpr std::string_view game_key = "y6E3c9"sv;
constexpr std::string_view base64_key = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"sv;

std::array<std::uint8_t, 28> challenge_enc(std::array<std::uint8_t, 20> challenge) {
    std::array<std::uint8_t, 28> result;
    std::array<std::uint8_t, 256> lookup;

    std::iota(lookup.begin(), lookup.end(), 0);
    std::uint8_t a = 0, b = 0;
    for (std::size_t i = 0; i < 256; ++i) {
        a += lookup[i] + game_key[i % game_key.size()];
        std::swap(lookup[i], lookup[a]);
    }

    a = 0;
    for (std::size_t i = 0; i < challenge.size(); ++i) {
        a += challenge[i] + 1;
        b += lookup[a];
        std::swap(lookup[a], lookup[b]);
        challenge[i] ^= lookup[(lookup[a] + lookup[b]) & 0xFF];
    }

    // enctype == 0: no-op obfuscation step

    // base64 encode result
    std::size_t i, j;
    for (i = j = 0; i < 18; i += 3) {
        result[j++] = base64_key[                                   (challenge[i    ] >> 2)];
        result[j++] = base64_key[((challenge[i    ] & 0x03) << 4) | (challenge[i + 1] >> 4)];
        result[j++] = base64_key[((challenge[i + 1] & 0x0F) << 2) | (challenge[i + 2] >> 6)];
        result[j++] = base64_key[ (challenge[i + 2] & 0x3F)                                ];
    }
    result[j++] = base64_key[                                   (challenge[i    ] >> 2)];
    result[j++] = base64_key[((challenge[i    ] & 0x03) << 4) | (challenge[i + 1] >> 4)];
    result[j++] = base64_key[((challenge[i + 1] & 0x0F) << 2)                          ];
    result[j++] = base64_key[0];

    return result;
}
