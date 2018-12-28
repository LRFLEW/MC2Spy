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

#include <unordered_map>

// There's no hash<udp::endpoint>, so here's a simple version assuming a specified v4 address
template<class E>
struct hash_endpoint_v4 {
    std::size_t operator()(const E &e) const {
        std::size_t hash = 0;
        boost::hash_combine(hash, e.address().to_v4().to_uint());
        boost::hash_combine(hash, e.port());
        return hash;
    }
};

class reporting_writer : public write_buffer {
public:
    void start_packet(std::uint8_t type, std::array<std::uint8_t, 4> userkey)
    { reset(); _send = false; array(_magic); uint8(type); array(userkey); }
    void finish() { _send = true; }
    void reset() { write_buffer::reset(); _send = false; }
    bool has_packet() { return _send; }

private:
    bool _send = false;
    static constexpr std::array<std::uint8_t, 2> _magic = { 0xFE, 0xFD };
};

class reporting;
class reportingcon {
public:
    reportingcon(reporting &main, const udp::endpoint &e) :
        _endpoint(e), _main(main) { }

    void handle_packet(read_buffer &read);

private:
    void handle_challenge(read_buffer &read);
    void handle_heartbeat(read_buffer &read);
    void handle_keepalive(read_buffer &read);

    udp::endpoint _endpoint;
    reporting &_main;

    std::array<std::uint8_t, 4> _userkey;
    bool _userkey_set = false;
    std::array<std::uint8_t, 20> _challenge;
    bool _sent_challenge = false;
    bool _registered = false;
    std::unordered_map<std::string, std::string> _serverkeys;
    std::unordered_map<std::string, std::vector<std::string>> _playerkeys;
};

class reporting {
public:
    reporting(asio::io_context &io_context);

private:
    void receive_one();
    void handle_receive(const error_code &error, std::size_t bytes_transferred);

    reporting_writer &write() { return _write; }
    friend reportingcon;

    udp::socket _socket;
    read_buffer _read;
    reporting_writer _write;
    udp::endpoint _endpoint;
    std::unordered_map<udp::endpoint, reportingcon, hash_endpoint_v4<udp::endpoint>> _connections;
};
