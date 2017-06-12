#include "Server.h"
#include "DataIO.h"
#include "Common.h"

#include <boost/asio.hpp>

#include <utility>
#include <vector>
#include <algorithm>
#include <iostream>


namespace network {

namespace 
{
    ClientID generateID() 
    {
        static ClientID idCounter = 0;
        return ++idCounter;
    }
}

//------------------------------------------------------------------------------

class Server::Impl 
{
    using Connection = boost::signals2::connection;
    using SocketPtr  = std::shared_ptr<boost::asio::ip::tcp::socket>;

    class Client 
    {
    public:
        Client(SocketPtr s) : socket(s), errorCount(0) 
        { clientID = generateID(); }

        ClientID   clientID;
        SocketPtr  socket;
        int        errorCount;
    };


public:

    Impl(Server* parent, unsigned port);
    ~Impl();

    void poll();

    void send(const std::string&);
    void send(const std::string&, ClientID);
    auto connectionCount() const -> size_t;

private:

    void connectionCount(size_t c)                  { _parent->_connectionCount(c); }
    void dataReceived(std::string s, ClientID c)    { _parent->_dataReceived(s, c); }
    void errorEmitted(std::string e)                { _parent->_errorEmitted(e); }

    void accept();
    void onDataReceived(SocketPtr socket, const std::string& data);
    void onSocketDisconnected(SocketPtr socket);
    void closeSocket(SocketPtr);
    void socketError(SocketPtr);

    Server* _parent;
    DataIO  _dataIO;

    boost::asio::io_service        _ioService;
    boost::asio::ip::tcp::acceptor _acceptor;
    std::vector<Client>            _clients;
};


//------------------------------------------------------------------------------
//--- Implementation
//------------------------------------------------------------------------------

Server::Impl::Impl(Server* parent, unsigned port)
: _parent(parent)
, _acceptor(_ioService, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
{
    _dataIO.connectSocketDisconnect([this](SocketPtr socket)                { onSocketDisconnected(socket); });
    _dataIO.connectDataReceived([this](SocketPtr socket, std::string data)  { onDataReceived(socket, data); });
    _dataIO.connectErrorEmitted([this](SocketPtr socket, std::string error) { socketError(socket); errorEmitted(error); });

    _acceptor.listen();
    accept();
}

//------------------------------------------------------------------------------

Server::Impl::~Impl()
{
    _acceptor.cancel();
    _acceptor.close();
    for (auto c : _clients) closeSocket(c.socket);
}

//------------------------------------------------------------------------------

void Server::Impl::poll() 
{ 
    _ioService.poll_one(); 
}

//---------------------------------------------------------------------

size_t Server::Impl::connectionCount() const 
{ 
    return _clients.size();
}

//------------------------------------------------------------------------------

void Server::Impl::closeSocket(SocketPtr socket)
{
    boost::system::error_code ec;
    socket->cancel();
    socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
    socket->close();

    auto newEnd = std::remove_if(_clients.begin(), _clients.end(),
                                 [socket](const Client& c){ return c.socket == socket; });
    _clients.erase(newEnd, _clients.end());
}

//------------------------------------------------------------------------------

void Server::Impl::socketError(SocketPtr socket)
{
    auto it = std::find_if(_clients.begin(), _clients.end(), [socket](const Client& c){ return c.socket == socket; });
    if (it != _clients.end())
    {
        if (++it->errorCount > cfg::failtureCountForDisconnect)
        {
            closeSocket(it->socket);
            connectionCount(connectionCount()); 
        }
    }
}

//------------------------------------------------------------------------------

void Server::Impl::accept()
{
    SocketPtr socket = std::make_shared<boost::asio::ip::tcp::socket>(_ioService);

    _acceptor.async_accept(*socket, [socket,this](boost::system::error_code error)
    {
        if (!error) {
            _clients.push_back( Client(socket) );
            connectionCount(connectionCount()); 
        }
        else {
            errorEmitted("Server: Error async_accept");
        }

        _dataIO.listen(socket);
        accept();
    });
}

//------------------------------------------------------------------------------

void Server::Impl::send(const std::string& data)
{
    for (auto& c : _clients)
    {
        _dataIO.send(c.socket, data);
    }
}

//------------------------------------------------------------------------------

void Server::Impl::send(const std::string& data, ClientID id)
{
    auto it = std::find_if(_clients.begin(), _clients.end(), [id](const Client& c){ return c.clientID == id; });
    if (it != _clients.end()) 
    {
        _dataIO.send(it->socket, data);
    }
}

//------------------------------------------------------------------------------

void Server::Impl::onDataReceived(SocketPtr socket, const std::string& data)
{
    auto it = std::find_if(_clients.begin(), _clients.end(), [socket](const Client& c){ return c.socket == socket; });
    if (it != _clients.end()) 
    {
        it->errorCount = 0;
        dataReceived(data, it->clientID);
    }
    _dataIO.listen(socket);
}

//------------------------------------------------------------------------------

void Server::Impl::onSocketDisconnected(SocketPtr socket)
{
    closeSocket(socket);
    connectionCount(connectionCount()); 
}


//------------------------------------------------------------------------------
//--- Server
//------------------------------------------------------------------------------

Server::Server(unsigned port)
: _impl(nullptr)
, _port(port)
{}

Server::~Server() { stop(); }

void Server::start()                                    { if (!_impl) _impl.reset(new Impl(this, _port)); }
void Server::stop()                                     { _impl.reset(nullptr); }

void Server::send(const std::string& data)              { if (_impl) _impl->send(data); }
void Server::send(const std::string& data, ClientID id) { if (_impl) _impl->send(data, id); }
void Server::poll()                                     { if (_impl) _impl->poll();  }
auto Server::started()         const -> bool            { return _impl != nullptr;       }
auto Server::connectionCount() const -> size_t          { return _impl ? _impl->connectionCount() : 0; }

//------------------------------------------------------------------------------

Server::Connection Server::connectConnectionCount(const std::function<void(size_t)> handler)
{ return _connectionCount.connect(handler); }

Server::Connection Server::connectErrorEmitted(const std::function<void(std::string)> handler)
{ return _errorEmitted.connect(handler); }

Server::Connection Server::connectDataReceived(const std::function<void(std::string, ClientID)> handler) 
{ return _dataReceived.connect(handler); }

//------------------------------------------------------------------------------

}// namespace

