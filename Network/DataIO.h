// Copyright (c) 2017  Mathias Roder (teuse@mailbox.org)

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
#pragma once

#include <boost/asio/ip/tcp.hpp>
#include <boost/circular_buffer.hpp>
#include <boost/signals2.hpp>

#include <string>
#include <vector>
#include <functional>
#include <memory>


namespace network {

//------------------------------------------------------------------------------

class DataIO 
{
    using DataBuffer  = std::vector<boost::asio::const_buffer>;
    using RingBuffer  = boost::circular_buffer<std::string>;
    using SocketPtr   = std::shared_ptr<boost::asio::ip::tcp::socket>;
    using Connection  = boost::signals2::connection;

public:

    DataIO();

    void send(SocketPtr, const std::string&);
    void listen(SocketPtr); // Not blocking

    // Callbacks 
    Connection connectSocketDisconnect(const std::function<void(SocketPtr)>);
    Connection connectDataReceived(const std::function<void(SocketPtr, std::string)>);
    Connection connectErrorEmitted(const std::function<void(SocketPtr, std::string)>);

private:

    void receiveData(SocketPtr socket, size_t size);
    auto createHeader(size_t dataSize)           -> std::string;
    auto dataSize(std::string headerData)        -> size_t;

    RingBuffer        _ringbuffer;
    std::vector<char> _receiveHeaderBuffer;
    std::vector<char> _receiveDataBuffer;

    boost::signals2::signal<void(SocketPtr)> _socketDisconnect;
    boost::signals2::signal<void(SocketPtr, std::string)> _dataReceived;
    boost::signals2::signal<void(SocketPtr, std::string)> _errorEmitted;
};


}


