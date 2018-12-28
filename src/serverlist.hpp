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
#include "slistenc.hpp"

#include <memory>

class serverlistcon : public std::enable_shared_from_this<serverlistcon> {
public:
    serverlistcon(asio::io_context &io_context) : _socket(io_context) { }
    tcp::socket &socket() { return _socket; }
    void read_message();

private:
    void recv_msg_len(const error_code &error);
    void recv_msg(const error_code &error);
    void write_msg(const error_code &error);

    bool listRequest();

    bool sent_list = false;

    tcp::socket _socket;
    read_buffer _read;
    slistenc _write;
};

class serverlist {
public:
    serverlist(asio::io_context &io_context);

private:
    void accept_one();
    void handle_accept(std::shared_ptr<serverlistcon> connection, const error_code &error);

    tcp::acceptor _acceptor;
};
