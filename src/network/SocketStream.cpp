#include "network/SocketStream.h"
#include <cstring>

#ifdef _WIN32
    static bool g_wsa_inited = false;
#endif

// ---------- 共用工具 ----------
static void throw_on(bool cond, const char* msg)
{
    if (cond) throw std::runtime_error(msg);
}

// ---------- Windows 专用 ----------
void SocketStream::InitWSA()
{
#ifdef _WIN32
    if (!g_wsa_inited) {
        WSADATA wsa;
        throw_on(WSAStartup(MAKEWORD(2, 2), &wsa) != 0, "WSAStartup failed");
        g_wsa_inited = true;
    }
#endif
}

// ---------- ctor / dtor ----------
SocketStream::SocketStream(const std::string& ip, uint16_t port)
{
    InitWSA();
    sock_ = ::socket(AF_INET, SOCK_STREAM, 0);
    throw_on(sock_ == -1 || sock_ == INVALID_SOCKET, "socket() failed");

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip.c_str());

    throw_on(::connect(sock_, (sockaddr*)&addr, sizeof(addr)) < 0,
             "connect() failed");
}

SocketStream::SocketStream(socket_t s) : sock_(s) { InitWSA(); }

SocketStream::~SocketStream()
{
#ifdef _WIN32
    closesocket(sock_);
#else
    close(sock_);
#endif
}

// ---------- send / recv ----------
void SocketStream::SendLine(const std::string& line) const
{
    std::string buf = line + "\n";
    const char* p  = buf.data();
    size_t left    = buf.size();
    while (left) {
        int n = ::send(sock_, p, static_cast<int>(left), 0);
        throw_on(n <= 0, "send() failed");
        left -= n;
        p    += n;
    }
}

std::string SocketStream::RecvLine() const
{
    std::string line;
    char ch;
    while (true) {
        int n = ::recv(sock_, &ch, 1, 0);
        if (n <= 0) return {};           // 对端关闭或出错
        if (ch == '\n') break;
        line.push_back(ch);
    }
    return line;
}
