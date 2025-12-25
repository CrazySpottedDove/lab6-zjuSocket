#include "server.h"
#include "zjuSocket/common.h"
#include "zjuSocket/log.h"
#include "zjuSocket/message.h"
#include <arpa/inet.h>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <iomanip>
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

void Server::send(socket_handle_t client_socket_handle, const Message* message)
{
    const auto error_code = ::send(client_socket_handle, message, sizeof(Message), 0);
    if (error_code < 0) {
        WARNING("Send message to client %d failed.\n\tError code: %zd",
                client_socket_handle,
                error_code);
    }
    else {
        INFO("Sent message to client %d.", client_socket_handle);
    }
}

void* Server::clientHandlerThread(void* arg)
{
    auto*           thread_arg           = static_cast<ThreadArg*>(arg);
    Server*         server               = thread_arg->server;
    socket_handle_t client_socket_handle = thread_arg->client_socket_handle;
    char            buffer[4096];
    size_t          buffer_len = 0;
    // send greeting as a proper Message so client interprets it correctly
    Message greeting_msg;
    greeting_msg.type = MessageType::REPOST;
    strncpy(greeting_msg.data, "Hello World!", sizeof(greeting_msg.data) - 1);
    server->send(client_socket_handle, &greeting_msg);

    bool    running = true;
    while (running) {
        ssize_t n = recv(client_socket_handle, buffer + buffer_len, sizeof(buffer) - buffer_len, 0);
        if (n <= 0) {
            WARNING("Receive message from client failed, maybe client disconnected.");
            break;
        }
        buffer_len += n;

        size_t offset = 0;
        while (buffer_len - offset >= sizeof(Message)) {
            Message msg;
            memcpy(&msg, buffer + offset, sizeof(Message));
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
                INFO("Client %d requested current time.", client_socket_handle);
                time_t now;
                time(&now);
                struct tm* local_time = localtime(&now);
                char       time_str[100];
                strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", local_time);
                Message response_msg;
                response_msg.type = MessageType::ANSWER_GET_TIME;
                strncpy(response_msg.data, time_str, sizeof(response_msg.data));
                server->send(client_socket_handle, &response_msg);
                break;
            }
            case MessageType::GET_NAME:
            {
                INFO("Client %d requested server name.", client_socket_handle);
                Message response_msg;
                response_msg.type = MessageType::ANSWER_GET_NAME;
                gethostname(response_msg.data, sizeof(response_msg.data));
                server->send(client_socket_handle, &response_msg);

                break;
            }
            case MessageType::GET_CLIENT_LIST:
            {
                INFO("Client %d requested client list.", client_socket_handle);
                server->clients_mutex_.lock();
                std::string client_list;
                for (const auto& client : server->clients_) {
                    client_list += client.ip_address + ":" + std::to_string(client.port) + "\n";
                }
                server->clients_mutex_.unlock();
                Message response_msg;
                response_msg.type = MessageType::ANSWER_GET_CLIENT_LIST;
                strncpy(response_msg.data, client_list.c_str(), sizeof(response_msg.data));
                server->send(client_socket_handle, &response_msg);

                break;
            }
            case MessageType::SEND_MESSAGE:
            {
                {
                    std::string data = std::string(msg.data);
                    std::string ip   = data.substr(0, data.find(":"));
                    data             = data.substr(data.find(":") + 1);
                    int port         = stoi(data.substr(0, data.find(":")));
                    data             = data.substr(data.find(":") + 1);
                    INFO("Client %d sending message to %s:%d",
                         client_socket_handle,
                         ip.c_str(),
                         port);
                    int target = -1;
                    server->clients_mutex_.lock();
                    auto& clients = server->clients_;
                    // 查找目标客户端
                    for (size_t i = 0; i < clients.size(); i++) {
                        if (clients[i].ip_address == ip && clients[i].port == port) {
                            target = i;
                            break;
                        }
                    }
                    Message answer_message;
                    answer_message.type = MessageType::ANSWER_SEND_MESSAGE;
                    if (target == -1) {   // 没有目标
                        server->clients_mutex_.unlock();
                        WARNING("No such client %s:%d", ip.c_str(), port);
                        // 回复发送者，目标不存在
                        snprintf(answer_message.data,
                                 sizeof(answer_message.data),
                                 "%s",
                                 ANSWER_SEND_MESSAGE_NOT_FOUND);
                        server->send(client_socket_handle, &answer_message);
                    }
                    else {
                        Message forward_message;
                        forward_message.type = MessageType::REPOST;
                        strcpy(forward_message.data, data.c_str());
                        // 转发消息到目标客户端
                        server->send(clients[target].handle, &forward_message);
                        server->clients_mutex_.unlock();
                        // 回复发送者，发送成功
                        snprintf(answer_message.data,
                                 sizeof(answer_message.data),
                                 "%s",
                                 ANSWER_SEND_MESSAGE_OK);
                        server->send(client_socket_handle, &answer_message);
                    }

                    break;
                }
            }
            default:
                WARNING("Unknown message type received from client %d.", client_socket_handle);
                break;
            }
            offset += sizeof(Message);
        }

        // 移动剩余的数据到缓冲区的开头
        if (offset < buffer_len) {
            memmove(buffer, buffer + offset, buffer_len - offset);
        }
        buffer_len -= offset;
    }
    delete thread_arg;
    return nullptr;
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
        auto*     thread_arg = new ThreadArg{this, client_socket_handle};
        pthread_t client_thread_id;
        pthread_create(&client_thread_id, nullptr, clientHandlerThread, thread_arg);
        pthread_detach(client_thread_id);
    }
}
