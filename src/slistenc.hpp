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

#pragma once

#include "common.hpp"
#include "buffer.hpp"

#include <algorithm>

class slistenc : public write_buffer {
public:
    void init(std::array<std::uint8_t, 8> clientkey);

protected:
    virtual void write(const void *data, std::size_t size) override;

private:
    std::uint8_t enc(std::uint8_t v);

    std::array<std::uint8_t, 256> lookup;
    std::uint8_t a, b, c, d, e;
    bool override_enc = false;
};
