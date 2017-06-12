#include "DataIO.h"

#include <boost/asio.hpp>

#include <iomanip>


namespace network {

//------------------------------------------------------------------------------

namespace cfg {
    static constexpr size_t headerLength   = 8;
    static constexpr size_t ringbufferSize = 50;
}

//------------------------------------------------------------------------------

DataIO::DataIO() 
: _ringbuffer(cfg::ringbufferSize)
{ }

//------------------------------------------------------------------------------

DataIO::Connection DataIO::connectDataReceived(const std::function<void(SocketPtr, std::string)> handler) 
{ return _dataReceived.connect(handler); }

DataIO::Connection DataIO::connectErrorEmitted(const std::function<void(SocketPtr, std::string)> handler) 
{ return _errorEmitted.connect(handler); }

DataIO::Connection DataIO::connectSocketDisconnect(const std::function<void(SocketPtr)> handler)
{ return _socketDisconnect.connect(handler); }

//------------------------------------------------------------------------------

void DataIO::send(DataIO::SocketPtr socket, const std::string& data)
{
    auto header = createHeader(data.size()); 

    DataBuffer buffer;
    _ringbuffer.push_back(header);
    buffer.push_back(boost::asio::buffer(_ringbuffer.back()));
    _ringbuffer.push_back(data);
    buffer.push_back(boost::asio::buffer(_ringbuffer.back()));

    boost::asio::async_write(*socket, buffer,
            [socket,this](boost::system::error_code er, std::size_t )
    {
        if (er) 
            _errorEmitted(socket, "DataIO: sending failed!");
    });
}

//------------------------------------------------------------------------------

void DataIO::listen(DataIO::SocketPtr socket)
{
    _receiveHeaderBuffer.resize(cfg::headerLength);

    boost::asio::async_read(*socket, boost::asio::buffer(_receiveHeaderBuffer),
            [socket,this](const boost::system::error_code &ec, std::size_t )
    {
        if (ec) {
            _errorEmitted(socket, "DataIO: receiving header failed!");
        }
        else {
            auto headerStr = std::string(&_receiveHeaderBuffer[0], _receiveHeaderBuffer.size());
            auto size  = dataSize(headerStr);
            if (size > 0) {
                receiveData(socket, size);
            }
            else {
                _errorEmitted(socket, "DataIO: received invalid header");
            }
        }
    });

}

//------------------------------------------------------------------------------

void DataIO::receiveData(DataIO::SocketPtr socket, size_t size)
{
    _receiveDataBuffer.resize(size);

    boost::asio::async_read(*socket, boost::asio::buffer(_receiveDataBuffer),
                        [socket, this](const boost::system::error_code &ec, std::size_t )
    {
        if (boost::asio::error::eof == ec) {
            _errorEmitted(socket, "DataIO: error::eof");
            _socketDisconnect(socket);
        }
        else if (boost::asio::error::connection_reset == ec) {
            _errorEmitted(socket, "DataIO: connection_reset");
            _socketDisconnect(socket);
        }
        else if (ec) {
            _errorEmitted(socket, "DataIO: receiving data failed!");
        }
        else
        {
            std::string dataString(&_receiveDataBuffer[0], _receiveDataBuffer.size());
            _dataReceived(socket, dataString);
        }
    });
}

//------------------------------------------------------------------------------

std::string DataIO::createHeader(size_t dataSize)
{
    std::ostringstream ostream;
    ostream << std::setw(cfg::headerLength) << std::hex << dataSize;
    return ostream.str();
}

//------------------------------------------------------------------------------

size_t DataIO::dataSize(std::string header)
{
    std::istringstream is(header);
    size_t size = 0;
    is >> std::hex >> size;
    return size;
}


}
