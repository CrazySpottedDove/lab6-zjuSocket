#pragma once
#include <cstdint>
#include <cstdio>
namespace zjuSocket {
enum class ClientCommand : uint8_t
{
    PrintHelp,
    ConnectServer,
    DisconnectServer,
    GetServerTime,
    GetServerName,
    GetClientList,
    SendMessage,
    Exit
};

// 不使用虚函数，而是使用模板实现零开销抽象
template<typename ClientType> class GenericClient
{
public:
    void Run()
    {
        handlePrintHelp();
        while (true) {
            int cmd;
            scanf("%d", &cmd);
            switch (static_cast<ClientCommand>(cmd)) {
            case ClientCommand::PrintHelp: handlePrintHelp(); break;
            case ClientCommand::ConnectServer: handleConnectServer(); break;
            case ClientCommand::DisconnectServer: handleDisconnectServer(); break;
            case ClientCommand::GetServerTime: handleGetServerTime(); break;
            case ClientCommand::GetServerName: handleGetServerName(); break;
            case ClientCommand::GetClientList: handleGetClientList(); break;
            case ClientCommand::SendMessage: handleSendMessage(); break;
            case ClientCommand::Exit:
                handleExit();
                // Can't reach here
            default:
                printf("Unknown Command!\n");
                handlePrintHelp();
                break;
            }
        }
    }

private:
    /**
     * @brief 检查是否已连接
     *
     * @return true
     * @return false
     */
    bool IsConnected() const { return static_cast<const ClientType*>(this)->IsConnected(); }

    /**
     * @brief 打印帮助信息
     *
     */
    void handlePrintHelp() { static_cast<ClientType*>(this)->handlePrintHelp(); }

    /**
     * @brief 连接到服务器
     *
     */
    void handleConnectServer() { static_cast<ClientType*>(this)->handleConnectServer(); }

    /**
     * @brief 断开与服务器的连接
     *
     */
    void handleDisconnectServer() { static_cast<ClientType*>(this)->handleDisconnectServer(); }

    /**
     * @brief 获取服务器时间
     *
     */
    void handleGetServerTime() { static_cast<ClientType*>(this)->handleGetServerTime(); }

    /**
     * @brief 获取服务器名称
     *
     */
    void handleGetServerName() { static_cast<ClientType*>(this)->handleGetServerName(); }

    /**
     * @brief 获取在线的客户端列表
     *
     */
    void handleGetClientList() { static_cast<ClientType*>(this)->handleGetClientList(); }

    /**
     * @brief 发送一个信息给其它客户端
     *
     */
    void handleSendMessage() { static_cast<ClientType*>(this)->handleSendMessage(); }

    /**
     * @brief 断开连接，并退出客户端
     *
     */
    [[noreturn]] void handleExit() { static_cast<ClientType*>(this)->handleExit(); }
};
}   // namespace zjuSocket