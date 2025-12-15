#pragma once

#include "zjuSocket/genericServer.h"

namespace zjuSocket {
class Server : public GenericServer<Server>
{
public:
    Server();
    void Run();
private:
    
};
}   // namespace zjuSocket