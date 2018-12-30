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

#include "reporting.hpp"

#include "qrenc.hpp"

#include <random>

constexpr std::size_t max_mtu = 1400;
constexpr std::string_view alphanumeric = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"sv;

static std::default_random_engine rng(std::random_device{}());
template<std::size_t N>
static std::array<std::uint8_t, N> random_printable() {
    std::array<std::uint8_t, N> arr;
    std::uniform_int_distribution<std::size_t> dist(0, alphanumeric.size() - 1);
    std::generate(arr.begin(), arr.end(),
        [&dist]() -> std::uint8_t { return alphanumeric[dist(rng)]; });
    return arr;
}

enum MsgType : std::uint8_t {
    Msg_Challenge = 1,
    Msg_Heartbeat = 3,
    Msg_KeepAlive = 8,
    Msg_ClientRegistered = 10,
};

void reportingcon::handle_packet(read_buffer &read) {
    std::uint8_t type = read.uint8();
    if (_userkey_set) {
        if (read.array<4>() != _userkey) return;
    } else {
        if (type != Msg_Heartbeat) return;
        _userkey = read.array<4>();
        _userkey_set = true;
    }

    switch (type) {
    case Msg_Challenge:
        handle_challenge(read);
        break;
    case Msg_Heartbeat:
        handle_heartbeat(read);
        break;
    case Msg_KeepAlive:
        break;
    }
}

void reportingcon::handle_challenge(read_buffer &read) {
    if (!_sent_challenge || _registered) return;

    std::array<std::uint8_t, 28> response = read.array<28>();
    if (read.uint8() != 0) return;
    if (!read.finished()) return;
    if (response != challenge_enc(_challenge)) return;

    _registered = true;
    reporting_writer &write = _main.write();
    write.start_packet(Msg_ClientRegistered, _userkey);
    write.finish();
}

void reportingcon::handle_heartbeat(read_buffer &read) {
    // TODO: avoid string copies in converting string_view to string
    // server keys
    for (std::string_view key; !(key = read.string()).empty(); ) {
        std::string_view value = read.string();
        _serverkeys[std::string(key)] = std::string(value);
    }
    if (_serverkeys["gamename"] != "mclub2pc"sv) return;
    if (_serverkeys["statechanged"] == "2"sv) return;

    // player keys
    std::uint16_t player_count = read.le_uint16();
    std::vector<std::string> keys;
    for (std::string_view key; !(key = read.string()).empty(); ) {
        std::string skey(key);
        if (_playerkeys.count(skey)) return;
        keys.push_back(skey);
        _playerkeys.insert({ skey, std::vector<std::string>(player_count) });
    }
    for (std::uint16_t i = 0; i < player_count; ++i) {
        for (const std::string &key : keys) {
            _playerkeys.at(key)[i] = read.string();
        }
    }

    // ends with 3 null bytes?
    if (read.array<3>() != std::array<std::uint8_t, 3>{ 0, 0, 0 }) return;
    if (!read.finished()) return;

    if (!_sent_challenge) {
        reporting_writer &write = _main.write();
        _challenge = random_printable<20>();
        write.start_packet(Msg_Challenge, _userkey);
        write.array(_challenge);
        write.uint8(0); // null terminator
        write.finish();
    }
}

void reportingcon::handle_keepalive(read_buffer &read) {
    // no msg body, so just ignore length

    // ping back?
    reporting_writer &write = _main.write();
    write.start_packet(Msg_KeepAlive, _userkey);
    write.finish();
}

reporting::reporting(asio::io_context &io_context) :
    _socket(io_context, udp::endpoint(udp::v4(), 27900)) {
    receive_one();
}

void reporting::receive_one() {
    _socket.async_receive_from(_read.buf(max_mtu), _endpoint,
        std::bind(&reporting::handle_receive, this, bind_error, bind_bytes_transferred));
}

void reporting::handle_receive(const error_code &error, std::size_t bytes_transferred) {
    if (!error) {
        _read.resize(bytes_transferred);
        _write.reset();
        if (_connections.count(_endpoint)) _connections.at(_endpoint).handle_packet(_read);
        else _connections.insert({ _endpoint, { *this, _endpoint } }).first->second.handle_packet(_read);
        if (_write.has_packet()) {
            _socket.async_send_to(_write.buf(), _endpoint, std::bind(&reporting::receive_one, this));
            // wait for write to read another packet to keep contents of _write
        } else receive_one();
    } else receive_one();
}
