#pragma once

#include <string>
namespace zjuSocket {
using socket_handle_t = int;
using port_t          = unsigned short;
struct ClientInfo
{
    socket_handle_t socket_handle;
    std::string     client_ip_address;
    port_t          client_port;
};
}   // namespace zjuSocket