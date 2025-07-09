#pragma once
/**
 * @file SocketStream.h
 * @brief 轻量级 TCP 流封装，发送/接收以 '\n' 结尾的 JSON 文本
 */
#include <string>
#include <stdexcept>
#include <cstdint>        // ★★★ 新增：uint16_t

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
    /* ---------- 构造 / 析构 ---------- */
    /// @note 客户端：主动连接
    SocketStream(const std::string& ip, uint16_t port);
    /// @note 服务器端：已接受的原生 socket
    explicit SocketStream(socket_t s);
    ~SocketStream();

    SocketStream(const SocketStream&)            = delete;
    SocketStream& operator=(const SocketStream&) = delete;

    /* ---------- 基础收发 ---------- */
    void SendLine(const std::string& line) const;   // 追加 '\n'
    std::string RecvLine() const;                   // 读到 '\n'（不含）

    socket_t Raw() const { return sock_; }

    /* ---------- 平台初始化 ---------- */
    static void InitWSA();        // ★★★ 改为 public：供外部 main 调用

private:
    socket_t sock_;
};
