#include "server.h"
#include "zjuSocket/common.h"
#include "zjuSocket/log.h"
#include "zjuSocket/message.h"
#include <arpa/inet.h>
#include <cstring>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/socket.h>
#include <unistd.h>
using namespace zjuSocket;

Server::Server()
{
    INFO("Server initializing...");
    // 创建服务器套接字，使用IPv4和TCP协议
    server_socket_handle_ = socket(AF_INET, SOCK_STREAM, 0);
    memset(&server_addr_, 0, sizeof(server_addr_));
    // 设置地址族为IPv4
    server_addr_.sin_family = AF_INET;
    // 绑定到所有可用的网络接口
    server_addr_.sin_addr.s_addr = htonl(INADDR_ANY);
    // 设置监听端口为 SERVER_PORT
    server_addr_.sin_port = htons(SERVER_PORT);
    // 绑定套接字到指定的IP地址和端口号
    if (bind(server_socket_handle_, (struct sockaddr*)&server_addr_, sizeof(server_addr_)) < 0) {
        ERROR("Bind server socket failed!");
        exit(1);
    }
    listen(server_socket_handle_, MAX_WAITING_CLIENT_COUNT);
    INFO("Server initialized.");
}

void* Server::ClientHandlerThread(void* arg)
{
    auto*           thread_arg           = static_cast<ThreadArg*>(arg);
    Server*         server               = thread_arg->server;
    socket_handle_t client_socket_handle = thread_arg->client_socket_handle;
    constexpr char  message[]            = "Hello World!";
    send(client_socket_handle, message, sizeof(message), 0);

    Message msg;
    bool    running = true;
    while (running) {
        // 尝试接收客户端信息
        if (recv(client_socket_handle, &msg, sizeof(Message), 0) < 0) {
            WARNING("Receive message from client failed, maybe client disconnected.");
            continue;
        }

        switch (msg.type) {
        case MessageType::DISCONNECT:
        {
            INFO("Client %d required disconnected.", client_socket_handle);

            server->clients_mutex_.lock();
            // 从客户端列表中移除该客户端
            for (size_t i = 0; i < server->clients_.size(); ++i) {
                if (server->clients_[i].handle == client_socket_handle) {
                    server->clients_.erase(server->clients_.begin() + i);
                    break;
                }
            }
            server->clients_mutex_.unlock();
            close(client_socket_handle);
            INFO("Client %d disconnected.", client_socket_handle);
            running = false;
            break;
        }
        case MessageType::GET_TIME:
        {

        }
        }
    }
}

void Server::Run()
{
    INFO("Server running...");
    while (true) {
        sockaddr_in client_addr;
        socklen_t   client_addr_len = sizeof(client_addr);
        INFO("Waiting for client connection...");
        // 等待一个客户端连接请求
        const socket_handle_t client_socket_handle =
            accept(server_socket_handle_, (struct sockaddr*)&client_addr, &client_addr_len);

        if (client_socket_handle < 0) {
            WARNING("Accept client connection failed!");
            continue;
        }

        SocketInfo client_socket_info;
        client_socket_info.handle     = client_socket_handle;
        client_socket_info.ip_address = inet_ntoa(client_addr.sin_addr);
        client_socket_info.port       = ntohs(client_addr.sin_port);
        clients_.push_back(std::move(client_socket_info));
        INFO("Client connected from %s:%d",
             client_socket_info.ip_address.c_str(),
             client_socket_info.port);

        // 创建一个线程来处理该客户端的通信
    }
}
