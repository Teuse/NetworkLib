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

#include <boost/signals2.hpp>

#include <string>
#include <functional>
#include <memory>


namespace network {

//------------------------------------------------------------------------------

using ClientID = unsigned long long;

class Server 
{
    using Connection = boost::signals2::connection;

public:
    Server(unsigned port);
    ~Server();

    // Run processing loop and execute read handler
    void poll();

    // Start/Stop the Server
    void start();
    void stop();

    auto started()         const -> bool;
    auto connectionCount() const -> size_t;

    // Send data to the clients
    void send(const std::string& data); // broadcast
    void send(const std::string& data, ClientID clientID);

    // Callbacks
    Connection connectConnectionCount(const std::function<void(size_t)>);
    Connection connectDataReceived(const std::function<void(std::string, ClientID)>);
    Connection connectErrorEmitted(const std::function<void(std::string)>);


private:

    class Impl; friend Impl;
    std::unique_ptr<Impl> _impl;

    unsigned _port;

    boost::signals2::signal<void(size_t)>                _connectionCount;
    boost::signals2::signal<void(std::string, ClientID)> _dataReceived;
    boost::signals2::signal<void(std::string)>           _errorEmitted;
};

//------------------------------------------------------------------------------

}

