#include "client.h"
#include <unistd.h>
constexpr char SERVER_IP[] = "10.162.106.169";
constexpr int SERVER_PORT = 4178;
int main() {
    zjuSocket::Client client;
    client.handleConnectServerCore(SERVER_IP, SERVER_PORT);
    client.handleGetServerTime();
    sleep(10);
    client.handleDisconnectServer();
    return 0;
}