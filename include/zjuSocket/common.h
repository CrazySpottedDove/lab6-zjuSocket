#pragma once

#include <string>
namespace zjuSocket {
using socket_handle_t = int;
using port_t          = unsigned short;
struct SocketInfo
{
    socket_handle_t handle;
    std::string     ip_address;
    port_t          port;
};
}   // namespace zjuSocket