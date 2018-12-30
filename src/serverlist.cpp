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

#include "serverlist.hpp"

#include "slistenc.hpp"

#define expect(X, Y) if ((X) != (Y)) return false

void serverlistcon::read_message() {
    asio::async_read(_socket, _read.buf(2),
        std::bind(&serverlistcon::recv_msg_len, shared_from_this(), bind_error));
}

void serverlistcon::recv_msg_len(const error_code &error) {
    try {
        if (error) return;
        std::uint16_t size = _read.be_uint16();
		if (size <= 2) return;
        asio::async_read(_socket, _read.buf(size - 2),
            std::bind(&serverlistcon::recv_msg, shared_from_this(), bind_error));
    } catch (const msg_bad_read &) { _socket.close(); }
}

enum MsgType : std::uint8_t {
    Msg_List = 0,
};


void serverlistcon::recv_msg(const error_code &error) {
    try {
        if (error) return;
        std::uint8_t type = _read.uint8();
        bool good;

        switch (type) {
        case Msg_List:
            good = listRequest(); break;
        }

        if (!good) _socket.close();
    } catch (const msg_bad_read &) { _socket.close(); }
}

void serverlistcon::write_msg(const error_code &error) {
    if (error) _socket.close();
    read_message();
}

bool serverlistcon::listRequest() {
    if (sent_list) return false;
    sent_list = true;

    expect(_read.uint8(), 1); // list version
    expect(_read.uint8(), 3); // encoding version
    expect(_read.le_uint32(), 34996503); // game protover
    expect(_read.string(), "mclub2pc"sv); // query game
    expect(_read.string(), "mclub2pc"sv); // encrypt game
    std::array<std::uint8_t, 8> clientkey = _read.array<8>();
    std::string_view filter = _read.string();
    //boost::string_view fields = _read.string();
	std::string_view fields = R"(\hostname\gamemode\numplayers\maxplayers\city\type\hook\race\team\tod\weather\powerups\cars\haspass\protover)"sv;
	expect(_read.string(), fields);
	if (fields.empty() || fields[0] != '\\') return false;
    expect(_read.be_uint32(), 4); // options = PUSH_UPDATES
    if (!_read.finished()) return false;

    _write.init(clientkey);
    _write.array(_socket.remote_endpoint().address().to_v4().to_bytes());
    _write.be_uint16(6500); // query port?
    std::size_t fields_elements = std::count(fields.begin(), fields.end(), '\\');
    if (fields_elements > 65535) return false;
    _write.le_uint16(static_cast<std::uint16_t>(fields_elements));

    for (std::string_view v = fields; !v.empty(); ) {
        std::size_t off = std::find(v.begin() + 1, v.end(), '\\') - v.begin();
        _write.string(v.substr(1, off - 1));
        _write.uint8(0);
        v = v.substr(off);
    }

    // Test server to list
	/*
	_write.uint8(0b01010100);
	_write.le_uint32(0x7F000001); // loopback address 127.0.0.1
	_write.le_uint16(10000); // random port
	_write.uint8(0xFF);
	_write.string("Testing"sv);
	_write.uint8(0xFF);
	_write.string("openstaging"sv);
	_write.uint8(0xFF);
	_write.string("1"sv);
	_write.uint8(0xFF);
	_write.string("8"sv);
	_write.uint8(0xFF);
	_write.string("2"sv);
	_write.uint8(0xFF);
	_write.string("4"sv);
	_write.uint8(0xFF);
	_write.string("-1"sv);
	_write.uint8(0xFF);
	_write.string("-1"sv);
	_write.uint8(0xFF);
	_write.string("-1"sv);
	_write.uint8(0xFF);
	_write.string("1"sv);
	_write.uint8(0xFF);
	_write.string("0"sv);
	_write.uint8(0xFF);
	_write.string("0"sv);
	_write.uint8(0xFF);
	_write.string("0"sv);
	_write.uint8(0xFF);
	_write.string("0"sv);
	_write.uint8(0xFF);
	_write.string("34996503"sv);
	 */

    _write.uint8(0);
    _write.le_uint32(0xFFFFFFFF);

    // Push keys? None known for MC2 at least
	// TODO: implement if necessary
	// netcap doesn't match OpenSpy source

    asio::async_write(_socket, _write.buf(), std::bind(&serverlistcon::write_msg, shared_from_this(), bind_error));
    return true;
}


serverlist::serverlist(asio::io_context &io_context) :
    _acceptor(io_context, tcp::endpoint(tcp::v4(), 28910)) {
    accept_one();
}

void serverlist::accept_one() {
    auto connection = std::make_shared<serverlistcon>(_acceptor.get_executor().context());
    _acceptor.async_accept(connection->socket(), 
        std::bind(&serverlist::handle_accept, this, connection, bind_error));
}

void serverlist::handle_accept(std::shared_ptr<serverlistcon> connection, const error_code& error) {
    if (!error) connection->read_message();
    accept_one();
}
