#include "client.h"
#include "zjuSocket/log.h"
#include "zjuSocket/message.h"

#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

namespace zjuSocket {

Client::Client()
    : server_socket_handle_(-1)
    , connected_(false)
{}

Client::~Client()
{
    if (connected_) {
        handleDisconnectServer();
    }
}

bool Client::IsConnected() const
{
    return connected_;
}

void Client::handlePrintHelp()
{
    printf("--- Client Help ---\n");
    printf("0: PrintHelp\n");
    printf("1: ConnectServer\n");
    printf("2: DisconnectServer\n");
    printf("3: GetServerTime\n");
    printf("4: GetServerName\n");
    printf("5: GetClientList\n");
    printf("6: SendMessage\n");
    printf("7: Exit\n");
}

void Client::handleConnectServer()
{
    if (connected_) {
        INFO("Already connected to server.");
        return;
    }

    char ip[128];
    int  port;
    printf("Server IP: ");
    scanf("%s", ip);
    printf("Server Port: ");
    scanf("%d", &port);
    INFO("Trying to connect to %s:%d", ip, port);

    // 创建套接字
    server_socket_handle_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_handle_ < 0) {
        ERROR("Create socket failed.");
        return;
    }

    // 填充服务器地址结构体
    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port   = htons(static_cast<uint16_t>(port));
    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0) {
        ERROR("Invalid server IP address.");
        close(server_socket_handle_);
        server_socket_handle_ = -1;
        return;
    }

    // 连接服务器
    if (connect(server_socket_handle_, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        ERROR("Connect to server %s:%d failed.", ip, port);
        close(server_socket_handle_);
        server_socket_handle_ = -1;
        return;
    }

    connected_ = true;
    INFO("Connected to server %s:%d", ip, port);

    // 启动接收线程，异步接收服务器消息
    pthread_create(&receiver_thread_, nullptr, receiverThread, this);
    pthread_detach(receiver_thread_);
}

void Client::handleDisconnectServer()
{
    if (!connected_) {
        INFO("Not connected.");
        return;
    }

    // 构造断开连接消息并发送
    Message msg;
    msg.type = MessageType::DISCONNECT;
    memset(msg.data, 0, sizeof(msg.data));
    ::send(server_socket_handle_, &msg, sizeof(msg), 0);

    // 关闭套接字
    close(server_socket_handle_);
    server_socket_handle_ = -1;
    connected_            = false;
    INFO("Disconnected from server.");
}

void Client::handleGetServerTime()
{
    if (!connected_) {
        WARNING("Not connected.");
        return;
    }
    // 构造请求时间消息并发送
    Message msg;
    msg.type = MessageType::GET_TIME;
    memset(msg.data, 0, sizeof(msg.data));
    ::send(server_socket_handle_, &msg, sizeof(msg), 0);
}

void Client::handleGetServerName()
{
    if (!connected_) {
        WARNING("Not connected.");
        return;
    }
    // 构造请求服务器名称消息并发送
    Message msg;
    msg.type = MessageType::GET_NAME;
    memset(msg.data, 0, sizeof(msg.data));
    ::send(server_socket_handle_, &msg, sizeof(msg), 0);
}

void Client::handleGetClientList()
{
    if (!connected_) {
        WARNING("Not connected.");
        return;
    }
    // 构造请求客户端列表消息并发送
    Message msg;
    msg.type = MessageType::GET_CLIENT_LIST;
    memset(msg.data, 0, sizeof(msg.data));
    ::send(server_socket_handle_, &msg, sizeof(msg), 0);
}

void Client::handleSendMessage()
{
    if (!connected_) {
        WARNING("Not connected.");
        return;
    }
    // 清除scanf遗留的换行符，避免影响后续输入
    int ch = getchar();
    while (ch != '\n' && ch != EOF) ch = getchar();

    char target[256];
    printf("Target (ip:port): ");
    if (!fgets(target, sizeof(target), stdin)) return;
    // 去除目标字符串末尾的换行符
    std::string target_s(target);
    if (!target_s.empty() && target_s.back() == '\n') target_s.pop_back();

    char message_buf[512];
    printf("Message: ");
    if (!fgets(message_buf, sizeof(message_buf), stdin)) return;
    // 去除消息字符串末尾的换行符
    std::string message_s(message_buf);
    if (!message_s.empty() && message_s.back() == '\n') message_s.pop_back();

    // 构造发送消息的数据包，格式为 "ip:port:message"
    Message msg;
    msg.type = MessageType::SEND_MESSAGE;
    memset(msg.data, 0, sizeof(msg.data));
    std::string payload = target_s + ":" + message_s;
    strncpy(msg.data, payload.c_str(), sizeof(msg.data) - 1);
    ::send(server_socket_handle_, &msg, sizeof(msg), 0);
}

[[noreturn]] void Client::handleExit()
{
    if (connected_) handleDisconnectServer();
    exit(0);
}

void* Client::receiverThread(void* arg)
{
    auto* client = static_cast<Client*>(arg);

    std::vector<char> buf;
    buf.reserve(64 * 1024);

    char tmp[4096];

    while (client->connected_) {
        // 接收服务器数据
        ssize_t n = ::recv(client->server_socket_handle_, tmp, sizeof(tmp), 0);
        if (n == 0) {
            // 服务器关闭连接
            WARNING("Server closed connection.");
            client->connected_ = false;
            break;
        }
        if (n < 0) {
            // EINTR为中断，继续接收；其他错误则断开
            if (errno == EINTR) continue;
            WARNING("recv error (errno=%d).", errno);
            client->connected_ = false;
            break;
        }

        // 将接收到的数据追加到缓冲区
        buf.insert(buf.end(), tmp, tmp + n);

        // 只要缓冲区中有完整的Message结构体就处理
        while (buf.size() >= sizeof(Message)) {
            Message msg;
            std::memcpy(&msg, buf.data(), sizeof(Message));

            // 移除已处理的数据
            buf.erase(buf.begin(), buf.begin() + sizeof(Message));

            // 根据消息类型处理不同的响应
            switch (msg.type) {
            case MessageType::REPOST: printf("[REPOST] %s\n", msg.data); break;
            case MessageType::ANSWER_GET_TIME: printf("[Server Time] %s\n", msg.data); break;
            case MessageType::ANSWER_GET_NAME: printf("[Server Name] %s\n", msg.data); break;
            case MessageType::ANSWER_GET_CLIENT_LIST:
            {
                printf("[Client List]\n");
                // 按行分割并打印客户端列表
                std::string list(msg.data);
                size_t      start = 0, end;
                while ((end = list.find('\n', start)) != std::string::npos) {
                    if (end > start) {
                        printf("%s\n", list.substr(start, end - start).c_str());
                    }
                    start = end + 1;
                }
                if (start < list.size()) {
                    printf("%s\n", list.substr(start).c_str());
                }
                break;
            }
            case MessageType::ANSWER_SEND_MESSAGE: printf("[Send Answer] %s\n", msg.data); break;
            default: printf("[Message Type %d] %s\n", static_cast<int>(msg.type), msg.data); break;
            }

            fflush(stdout);
        }
    }

    return nullptr;
}

}   // namespace zjuSocket