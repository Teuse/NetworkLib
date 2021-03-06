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
#include "Common.h"

#include <boost/signals2.hpp>

#include <string>
#include <functional>
#include <memory>


namespace network {

//------------------------------------------------------------------------------

class Client 
{
    using Connection = boost::signals2::connection;

public:

    Client(unsigned port);
    ~Client();
    
    // Run processing loop and execute read handler
    void poll();

    // (Dis-)Connect to a server
    void connect(std::string ip);
    void disconnect();

    auto connectionState() const -> ConnectionState;

    // Send data to Server
    void send(const std::string& data);

    // Callbacks
    Connection connectConnectionChanged(const std::function<void(ConnectionState)>);
    Connection connectDataReceived(const std::function<void(std::string)>);
    Connection connectErrorEmitted(const std::function<void(std::string)>);


private:

    void setState(ConnectionState state);


    class Impl; friend Impl;
    std::unique_ptr<Impl> _impl;

    unsigned _port;

    boost::signals2::signal<void(ConnectionState)> _connectionChanged;
    boost::signals2::signal<void(std::string)>     _dataReceived;
    boost::signals2::signal<void(std::string)>     _errorEmitted;
};

//------------------------------------------------------------------------------

}

