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

#include "buffer.hpp"

#include <algorithm>

void write_buffer::write(const void *data, std::size_t size) {
    std::size_t off = _data.size();
    _data.resize(off + size);
    std::copy_n(reinterpret_cast<const std::uint8_t *>(data), size, _data.begin() + off);
}

std::string_view read_buffer::string() {
    char *beg = reinterpret_cast<char *>(_buf.data()), *end = beg + _buf.size();
    char *ptr = std::find(beg, end, '\0');
    if (ptr == end) throw msg_bad_read();
    _buf = { (ptr + 1), static_cast<std::size_t>(end - (ptr + 1)) };
    return { beg, static_cast<std::size_t>(ptr - beg) };
}

const void *read_buffer::read(std::size_t size) {
    if (_buf.size() < size) throw msg_bad_read();
    const void *v = _buf.data();
    _buf += size;
    return v;
}
