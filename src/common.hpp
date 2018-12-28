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

#include <array>
#include <cstdint>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/endian/arithmetic.hpp>
#include <boost/functional/hash.hpp>
#include <boost/system/error_code.hpp>
#include <boost/utility/string_view.hpp>

namespace asio = boost::asio;
namespace endian = boost::endian;
namespace phs = asio::placeholders;

using asio::ip::tcp;
using asio::ip::udp;
using boost::system::error_code;

constexpr boost::string_view operator "" _sv(const char *str, std::size_t len) {
    return { str, len };
}
