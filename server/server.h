#pragma once

#include "zjuSocket/common.h"
#include "zjuSocket/genericServer.h"
#include "zjuSocket/message.h"
#include <mutex>
#include <netinet/in.h>
#include <vector>

namespace zjuSocket {
constexpr int SERVER_PORT              = 4178;
constexpr int MAX_WAITING_CLIENT_COUNT = 32;
class Server : public GenericServer<Server>
{
public:
    Server();
    void Run();

private:
    struct ThreadArg{
        Server* server;
        socket_handle_t client_socket_handle;
    };
    static void*            clientHandlerThread(void* arg);
    std::vector<SocketInfo> clients_;
    std::mutex              clients_mutex_;
    sockaddr_in             server_addr_;
    socket_handle_t         server_socket_handle_;
    void send(socket_handle_t client_socket_handle, const Message* message);
};
}   // namespace zjuSocket