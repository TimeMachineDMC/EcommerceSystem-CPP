#pragma once
#include <string>
#include <stdexcept>
#include <cstdint>       

#ifdef _WIN32
    #include <winsock2.h>
    #pragma comment(lib, "ws2_32.lib")
    using socket_t = SOCKET;
#else
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    using socket_t = int;
#endif

class SocketStream {
public:

    SocketStream(const std::string& ip, uint16_t port);

    explicit SocketStream(socket_t s);
    ~SocketStream();

    SocketStream(const SocketStream&)            = delete;
    SocketStream& operator=(const SocketStream&) = delete;

    void SendLine(const std::string& line) const;  
    std::string RecvLine() const;                  

    socket_t Raw() const { return sock_; }

    static void InitWSA();       

private:
    socket_t sock_;
};
