#pragma once

namespace zjuSocket {
template<typename ServerType> class GenericServer
{
public:
    /**
     * @brief 启动服务器，并保持监听用户端接入
     *
     */
    void Run(){
        static_cast<ServerType*>(this)->Run();
    }
};
}   // namespace zjuSocket