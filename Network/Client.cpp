#include "Client.h"
#include "DataIO.h"

#include <boost/asio.hpp>

#include <memory>
#include <iostream>


namespace network {

//------------------------------------------------------------------------------

class Client::Impl
{
    using Connection = boost::signals2::connection;
    using SocketPtr  = std::shared_ptr<boost::asio::ip::tcp::socket>;

public:

    Impl(Client* parent, unsigned port, std::string ip);
    ~Impl();

    void send(const std::string&);
    void poll()                                  { _ioService.poll_one(); }
    auto connectionState() const -> ConnectionState { return _state;     }

private:

    void connectionChanged(ConnectionState cs) { return _parent->_connectionChanged(cs); }
    void dataReceived(std::string r)           { return _parent->_dataReceived(r);       }
    void errorEmitted(std::string e)           { return _parent->_errorEmitted(e);       }

    void setState(ConnectionState state);
    void resolve(unsigned port, std::string ip);
    void connect(boost::asio::ip::tcp::resolver::iterator&);
    void onDataReceived(SocketPtr socket, const std::string& data);
    void onSocketDisconnected(SocketPtr socket);
    void socketError();
    void closeSocket();

    Client*         _parent;
    DataIO          _dataIO;
    ConnectionState _state;

    boost::asio::io_service        _ioService;
    boost::asio::ip::tcp::resolver _resolver;
    SocketPtr                      _socket;
    int                            _errorCount;
};


//------------------------------------------------------------------------------
//--- Implementation
//------------------------------------------------------------------------------

Client::Impl::Impl(Client* parent, unsigned port, std::string ip)
: _parent(parent)
, _resolver(_ioService)
{
    _dataIO.connectSocketDisconnect([this](SocketPtr socket)                { onSocketDisconnected(socket); });
    _dataIO.connectDataReceived([this](SocketPtr socket, std::string data)  { onDataReceived(socket, data);  });
    _dataIO.connectErrorEmitted([this](SocketPtr socket, std::string error) { ++_errorCount; errorEmitted(error); });

    _socket = std::make_shared<boost::asio::ip::tcp::socket>(_ioService);

    resolve(port, ip);
}

//------------------------------------------------------------------------------

Client::Impl::~Impl()
{
    _resolver.cancel();
    closeSocket();
    _ioService.run();
}

//------------------------------------------------------------------------------

void Client::Impl::setState(ConnectionState state)
{
    if (_state != state)
    {
        _state = state;
        connectionChanged(state);
    }
}

//------------------------------------------------------------------------------

void Client::Impl::socketError()
{
    if (++_errorCount > cfg::failtureCountForDisconnect)
    {
        closeSocket();
        setState(STATE_OFF);
    }
}

//------------------------------------------------------------------------------

void Client::Impl::closeSocket()
{
    boost::system::error_code ec;
    _socket->cancel();
    _socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
    _socket->close();
}

//------------------------------------------------------------------------------

void Client::Impl::resolve(unsigned port, std::string ip)
{
    setState(STATE_CONNECTING);

    using namespace boost::asio::ip;
    tcp::resolver::query query(ip, std::to_string(port), tcp::resolver::query::canonical_name);

    _resolver.async_resolve(query, [this](const boost::system::error_code &ec, tcp::resolver::iterator it)
    {
        if (!ec && it != tcp::resolver::iterator()) {
            connect(it);
        }
        else {
            errorEmitted("Client: do_resolve failed!");
        }
    });
}

//------------------------------------------------------------------------------

void Client::Impl::connect(boost::asio::ip::tcp::resolver::iterator& it)
{
    using namespace boost::asio::ip;
    _socket->async_connect(*it, [this,&it](const boost::system::error_code &ec) mutable
    {
        if (!ec) {
            setState(STATE_CONNECTED);
            _dataIO.listen(_socket);
        }
        else {
            errorEmitted("Client: do_connect failed!");
        }
    });
}

//------------------------------------------------------------------------------

void Client::Impl::send(const std::string& data)
{
    _dataIO.send(_socket, data);
}

//------------------------------------------------------------------------------

void Client::Impl::onDataReceived(SocketPtr, const std::string& data)
{
    _errorCount = 0;
    dataReceived(data);
    _dataIO.listen(_socket);
}

//------------------------------------------------------------------------------

void Client::Impl::onSocketDisconnected(SocketPtr socket)
{
    closeSocket();
    setState(STATE_OFF);
}


//------------------------------------------------------------------------------
//--- Client
//------------------------------------------------------------------------------

Client::Client(unsigned port)
: _impl(nullptr)
, _port(port)
{}

Client::~Client() { disconnect(); }

void Client::connect(std::string ip)                    { if (!_impl) _impl.reset(new Impl(this, _port, ip)); }
void Client::disconnect()                               { _impl.reset(nullptr); }

void Client::send(const std::string& data)              { if (_impl) _impl->send(data); }
auto Client::connectionState() const -> ConnectionState { return _impl ? _impl->connectionState() : STATE_OFF; }
void Client::poll()                                  
{ 
    if (_impl && connectionState() == STATE_OFF)   disconnect();
    if (_impl)                                     _impl->poll();  
}

//------------------------------------------------------------------------------

Client::Connection Client::connectConnectionChanged(const std::function<void(ConnectionState)> handler)
{ return _connectionChanged.connect(handler); }

Client::Connection Client::connectDataReceived(const std::function<void(std::string)> handler) 
{ return _dataReceived.connect(handler); }

Client::Connection Client::connectErrorEmitted(const std::function<void(std::string)> handler)
{ return _errorEmitted.connect(handler); }

//------------------------------------------------------------------------------

}// namespace

