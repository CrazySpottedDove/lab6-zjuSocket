#pragma once

#include "zjuSocket/common.h"
#include "zjuSocket/genericServer.h"
#include <mutex>
#include <vector>

namespace zjuSocket {
class Server : public GenericServer<Server>
{
public:
    Server();
    void Run();
private:
    std::vector<SocketInfo> clients_;
    std::mutex clientsMutex_;
};
}   // namespace zjuSocket