# NetworkLib
NetworkLib is a small and cross-platorm library written in C++. Using this library, you can easily create Server/Clients and send data back and forth.

**Details:**
* Cross-platform C++ Library
* Supports one Server and multiple Clients
* Sends data as strings and therefore supports JSON, XML, binary data, ...

### Build & Install
```
cmake ../NetworkLib
make
```
**Note:**
The *install* step is not implemented yet! Let me know if you need it :)

**Build for iOS**
For iOS, you need a cross compiled version of boost. See also https://github.com/faithfracture/Apple-Boost-BuildScript.
Put the __include__ and __lib__ directory of boost to "/path/to/your-project/../boost-ios".

cmake -DIOS=ON -DBOOST_IOS_ROOT="/path/to/your/boost_root/" ../NetworkLib
make

__BOOST_IOS_ROOT__ should contain the __include__and __lib__ directories with all boost sources/libs. 

### Documentation
Create a Server and receive data, status updates, errors
```cpp
#include <Network/Server.h>
#include <string>
#include <iostream>

void createServer(unsigned port)
{
    network::Server server(port);
    server.start();

    server.connectConnectionCount([this](size_t count)      
    { std::cout << "Count of connected Clients changed: " << count << std::endl; });
    server.connectErrorEmitted([](std::string error)        
    { std::cout << "Server error: " << error << std::endl; });
    server.connectDataReceived([this](std::string dataStr, network::ClientID id)  
    { std::cout << "Data received from Client " << id << std::endl; });
}
```
Create a Client and receive data, status updates, errors
```cpp
#include <Network/Client.h>
#include <string>
#include <iostream>

void createClient(unsigned port, std::string serverIP)
{
    network::Client client(port);

    client.connectConnectionChanged([this](network::ConnectionState state)
    { std::cout << "Connection state changed" << std::endl; }
    client.connectDataReceived([this](std::string msg)
    { std::cout << "Data received from Server " << id << std::endl; });
    client.connectErrorEmitted([](std::string error)
    { std::cout << "Server error: " << error << std::endl; });

    client.connect(serverIP);
}
```

To broadcast data all connected clients:
```cpp
auto data = std::string("Hallo clients!");
server.send(data);
```
Respectively, send data from client to server:
```cpp
auto data = std::string("Hallo server!");
client.send(data);
```

To run the internal event loop and execute the read handler, you must call the following function from your application loop:
```cpp
server.poll();
```
Respectively for the client:
```cpp
client.poll();
```

### Dependencies
* C++11
* Boost 1.64.0 or higher
