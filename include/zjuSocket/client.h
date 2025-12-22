#pragma once
#include "zjuSocket/genericClient.h"
#include "zjuSocket/common.h"
#include <pthread.h>

namespace zjuSocket {
class Client : public GenericClient<Client>
{
public:
    Client();
    ~Client();

    bool IsConnected() const;

    void handlePrintHelp();
    void handleConnectServer();
    void handleDisconnectServer();
    void handleGetServerTime();
    void handleGetServerName();
    void handleGetClientList();
    void handleSendMessage();
    [[noreturn]] void handleExit();

private:
    socket_handle_t server_socket_handle_;
    bool            connected_;
    pthread_t       receiver_thread_;

    static void* receiverThread(void* arg);
};
} // namespace zjuSocket
