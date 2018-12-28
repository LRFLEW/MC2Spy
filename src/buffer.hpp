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

#include <exception>
#include <iostream>
#include <iomanip>

class msg_bad_read : public std::out_of_range {
public:
    msg_bad_read() : std::out_of_range("Bad data found in msg_bad_read") { }
};

class write_buffer {
public:
    asio::const_buffer buf() const { return asio::buffer(_data); }
    void reset() { _data.clear(); }

    void uint8(std::uint8_t v) { write<std::uint8_t>(v); }
    void int8(std::int8_t v) { write<std::int8_t>(v); }
    void le_uint16(std::uint16_t v) { write<endian::little_uint16_t>(v); }
    void le_int16(std::int16_t v) { write<endian::little_int16_t>(v); }
    void be_uint16(std::uint16_t v) { write<endian::big_uint16_t>(v); }
    void be_int16(std::int16_t v) { write<endian::big_int16_t>(v); }
    void le_uint32(std::uint32_t v) { write<endian::little_uint32_t>(v); }
    void le_int32(std::int32_t v) { write<endian::little_int32_t>(v); }
    void be_uint32(std::uint32_t v) { write<endian::big_uint32_t>(v); }
    void be_int32(std::int32_t v) { write<endian::big_int32_t>(v); }
    template<std::size_t N> void array(const std::array<std::uint8_t, N> &v)
    { return write<const std::array<std::uint8_t, N> &>(v); }
    void string(boost::string_view v) { write(v.data(), v.size()); uint8(0); }

protected:
    // virtual to allow compression/encryption subclasses
    virtual void write(const void *data, std::size_t size);
    template<class T> void write(T v) { write(&v, sizeof(T)); }

    std::vector<std::uint8_t> _data;
};



class read_buffer {
public:
    asio::mutable_buffer buf(std::size_t size)
    { _msg.resize(size); _buf = asio::buffer(_msg); return _buf; };
    void resize(std::size_t size) { _msg.resize(size); _buf = asio::buffer(_msg); }
    bool finished() const noexcept { return _buf.size() == 0; }
    void skip(std::size_t size) { read(size); }

    std::uint8_t uint8() { return read<std::uint8_t>(); }
    std::int8_t int8() { return read<std::int8_t>(); }
    std::uint16_t le_uint16() { return read<endian::little_uint16_t>(); }
    std::int16_t le_int16() { return read<endian::little_int16_t>(); }
    std::uint16_t be_uint16() { return read<endian::big_uint16_t>(); }
    std::int16_t be_int16() { return read<endian::big_int16_t>(); }
    std::uint32_t le_uint32() { return read<endian::little_uint32_t>(); }
    std::int32_t le_int32() { return read<endian::little_int32_t>(); }
    std::uint32_t be_uint32() { return read<endian::big_uint32_t>(); }
    std::int32_t be_int32() { return read<endian::big_int32_t>(); }
    template<std::size_t N> std::array<std::uint8_t, N> array()
    { return read<std::array<std::uint8_t, N>>(); }
    boost::string_view string();

protected:
    const void *read(std::size_t size);
    template<class T> T read() { return *reinterpret_cast<const T *>(read(sizeof(T))); }

    std::vector<std::uint8_t> _msg;
    asio::mutable_buffer _buf;
};
